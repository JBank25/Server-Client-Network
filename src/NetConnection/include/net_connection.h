#pragma once

#include "net_common.h"
#include "net_log.h"
#include "net_message.h"
#include "net_log.h"
#include "net_server.h"
#include "net_tsqueue.h"


namespace net
{

	// Forward declaration
	template<typename T>
	class server_interface;

	/**
	 * @brief Connection class used by both clients and servers to send and receive data. 
	 * All the asio related code exists within this class. 
	 * 
	 * A unique socket exists for each connection. Connections can be "owned" by either
	 * a client or server. Messages coming in to the client or server are stored in their
	 * own queue, reference to this queue is used by the connection class. 
	 * This class handles validation of connections as well, process differs between clients
	 * and servers.
	 * 
	 * @tparam T - The type of data being used for communiction. Ex. 
	 * class CustomClient : public net::client_interface<CustomMsgTypes>
	 */
	template<typename T>
	class connection : public std::enable_shared_from_this<connection<T>>
	{
	public:
		// A connection is "owned" by either a server or a client, and its
		// behaviour is slightly different bewteen the two.
		enum class owner
		{
			server,
			client
		};

	private:
		// Used for logging 
		#define OWNER_TYPE_TO_STRING(o) (o == owner::server) ? "Server" : "Client"

	public:
		// Constructor: Specify Owner, connect to context, transfer the socket
		//				Provide reference to incoming message queue
		connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn)
			: m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn)
		{
			// TODO: error checking
			NET_CONNECTION_LOG_INFO("Being created, owner is: {0}", OWNER_TYPE_TO_STRING(parent))

			m_nOwnerType = parent;

			if(m_nOwnerType == owner::server)
			{
				// If the connection is Server -> Client, then get some random data
				// for to transform and send back for validation
				m_nHandshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
				
				// Calculate the result for checking when the client responds.
				m_nHandshakeCheck = scramble(m_nHandshakeOut);
			}
			else
			{
				// Connection is Client -> Server, nothing to define
				m_nHandshakeIn = 0;
				m_nHandshakeOut = 0;
			}
		}

		virtual ~connection()
		{}

		/**
		 * @brief Returns ID of connection. Used system wide, how clients will identify other
		 * clients across the whole system
		 * 
		 * @return uint32_t - connection id
		 */
		uint32_t GetID() const
		{
			return id;
		}

	public:
		/**
		 * @brief Updates the ID associated with the connection and attempts handshaking validation
		 * of client connection
		 * Only relevant if the owner is a server
		 * 
		 * @param server - ptr to server, used in ReadValidation to call OnClientConnected() after
		 * successful connection
		 * @param uid - new ID for the connection
		 */
		void ConnectToClient(server_interface<T>* server, uint32_t uid = 0)
		{
			if (m_nOwnerType == owner::server)
			{
				if (m_socket.is_open())
				{
					id = uid;
					NET_CONNECTION_LOG_TRACE("Server attempting to connect to client: {0}", uid)

					/**
					 * Client is attempting to connect. We want this client to validate itself first however.
					 * We will write out the handshake data to be validated to do this
					 */
					WriteValidation();

					// Next, issue task to sit and wait asynchronously for validation data to be sent back from client
					ReadValidation(server);
				}
			}
			else
			{
				NET_CONNECTION_LOG_ERROR("Client: {0} attempting to connect to client", OWNER_TYPE_TO_STRING(m_nOwnerType))
			}
		}

		/**
		 * @brief Attempts to connect client to server. ReadValidation begins the handshaking 
		 * validation process for the client
		 * 
		 * Relevant if the connection is a client

		 * @param endpoints - server endpoint the client socket will connect to for validation
		 */
		void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
		{
			// Only clients can connect to servers
			if (m_nOwnerType == owner::client)
			{
				// Request asio attempts to connect client socket to server endpoint
				asio::async_connect(m_socket, endpoints,
					[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
					{
						if (!ec)
						{
							// Handshaking with server has to happen
							ReadValidation();
						}
					});
			}
			else
			{
				NET_CONNECTION_LOG_ERROR("Server: {0} attempting to connect to server", OWNER_TYPE_TO_STRING(m_nOwnerType))
			}
		}

		/**
		 * @brief Can be called by both clients and servers and will shutdown the connection.
		 * 
		 */
		void Disconnect()
		{
			if (IsConnected())
				asio::post(m_asioContext, [this]() { m_socket.close(); });
		}

		/**
		 * @brief Is the connection open and currently valid
		 * 
		 * @return true if socket is open, else false
		 */
		bool IsConnected() const
		{
			return m_socket.is_open();
		}

		/**
		 * @brief Prime the connection to wait for incoming messages
		 * 
		 * TODO: Why did I create this function ??
		 * 
		 * @return 
		 */
		void StartListening()
		{
			
		}

	public:
		/**
		 * @brief ASYNC Send a message, connections are one-to-one so no need to specifiy
		 * the target, for a client, the target is the server and vice versa. Used by both
		 * clients and servers to send messages.
		 * 
		 * @param msg - msg to be sent
		 */
		void Send(const message<T>& msg)
		{
			asio::post(m_asioContext,
				[this, msg]()
				{
					// TODO: clarity on this comment
					// If the queue has a message in it, then we must 
					// assume that it is in the process of asynchronously being written.
					// Either way add the message to the queue to be output. If no messages
					// were available to be written, then start the process of writing the
					// message at the front of the queue.
					bool bWritingMessage = !m_qMessagesOut.empty();
					m_qMessagesOut.push_back(msg);
					if (!bWritingMessage)
					{
						WriteHeader();
					}
				});
		}



	private:
		/**
		 * @brief ASYNC - Prime context to write a message header
		 */
		void WriteHeader()
		{
			// If this function is called, we know the outgoing message queue must have 
			// at least one message to send. So allocate a transmission buffer to hold
			// the message, and issue the work - asio, send these bytes
			asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(message_header<T>)),
				[this](std::error_code ec, std::size_t length)
				{
					// asio has now sent the bytes - if there was a problem
					// an error would be available...
					if (!ec)
					{
						// ... no error, so check if the message header just sent also
						// has a message body...
						if (m_qMessagesOut.front().body.size() > 0)
						{
							// ...it does, so issue the task to write the body bytes
							WriteBody();
						}
						else
						{
							// ...it didnt, so we are done with this message. Remove it from 
							// the outgoing message queue
							m_qMessagesOut.pop_front();

							// If the queue is not empty, there are more messages to send, so
							// make this happen by issuing the task to send the next header.
							if (!m_qMessagesOut.empty())
							{
								WriteHeader();
							}
						}
					}
					else
					{
						// ...asio failed to write the message, we could analyse why but 
						// for now simply assume the connection has died by closing the
						// socket. When a future attempt to write to this client fails due
						// to the closed socket, it will be tidied up.
						std::cout << "[" << id << "] Write Header Fail.\n";
						m_socket.close();
					}
				});
		}

		/**
		 * @brief ASYNC - Prime context to write a message body
		 * 
		 */
		void WriteBody()
		{
			// If this function is called, a header has just been sent, and that header
			// indicated a body existed for this message. Fill a transmission buffer
			// with the body data, and send it!
			asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						// Sending was successful, so we are done with the message
						// and remove it from the queue
						m_qMessagesOut.pop_front();

						// If the queue still has messages in it, then issue the task to 
						// send the next messages' header.
						if (!m_qMessagesOut.empty())
						{
							WriteHeader();
						}
					}
					else
					{
						// Sending failed, see WriteHeader() equivalent for description :P
						std::cout << "[" << id << "] Write Body Fail.\n";
						m_socket.close();
					}
				});
		}

		/**
		 * @brief - ASYNC, primes our asio context to read in a message header. The data will be
		 * read from m_socket and will be of size sizeof(message_header<T>). If the message header
		 * indicates that the body size is > 0, the ReadBody function will be executed as well. If the
		 * message header indicates size == 0 then we will simply add the incoming message to the message
		 * queue.
		 */
		void ReadHeader()
		{
			// If this function is called, we are expecting asio to wait until it receives
			// enough bytes to form a header of a message. We know the headers are a fixed
			// size, so allocate a transmission buffer large enough to store it. In fact, 
			// we will construct the message in a "temporary" message object as it's 
			// convenient to work with.
			asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(message_header<T>)),
				[this](std::error_code ec, std::size_t length)
				{						
					if (!ec)
					{
						// A complete message header has been read, check if this message
						// has a body to follow...
						if (m_msgTemporaryIn.header.size > 0)
						{
							// ...it does, so allocate enough space in the messages' body
							// vector, and issue asio with the task to read the body.
							m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
							ReadBody();
						}
						else
						{
							// it doesn't, so add this bodyless message to the connections
							// incoming message queue
							AddToIncomingMessageQueue();
						}
					}
					else
					{
						// Reading form the client went wrong, most likely a disconnect
						// has occurred. Close the socket and let the system tidy it up later.
						// NET_CONNECTION_LOG_INFO("[CONNECTION] {0}", m_nOwnerType)
						NET_CONNECTION_LOG_INFO("[CONNECTION] {0} Read Header Fail", OWNER_TYPE_TO_STRING(m_nOwnerType))
						std::cout << "[" << id << "] Read Header Fail.\n";
						m_socket.close();
					}
				});
		}

		/**
		 * @brief - ASYNC, primes our asio context to read in a message body. The data will be
		 * read from m_socket and will be of size m_msgTemporaryIn.body.size(). The message will be
		 * added to the message queue assuming no errors occurred. If an error here occurs we just close
		 * the socket.
		 * 
		 */
		void ReadBody()
		{
			// If this function is called, a header has already been read, and that header
			// request we read a body, The space for that body has already been allocated
			// in the temporary message object, so just wait for the bytes to arrive...
			asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
				[this](std::error_code ec, std::size_t length)
				{						
					if (!ec)
					{
						// ...and they have! The message is now complete, so add
						// the whole message to incoming queue
						AddToIncomingMessageQueue();
					}
					else
					{
						// As above!
						std::cout << "[" << id << "] Read Body Fail.\n";
						m_socket.close();
					}
				});
		}

		/**
		 * @brief Adds messages to connection's message queue.
		 * If the connection is associated with a server then we will tag the message
		 * with a shared ptr to the connection. 
		 * If the connection is associated with a client we can tag it with a nullptr.
		 * 
		 */
		void AddToIncomingMessageQueue()
		{				
			// Shove it in queue, converting it to an "owned message", by initialising
			// with the a shared pointer from this connection object
			if(m_nOwnerType == owner::server)
				m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
			else
				m_qMessagesIn.push_back({ nullptr, m_msgTemporaryIn });

			// We must now prime the asio context to receive the next message. It 
			// wil just sit and wait for bytes to arrive, and the message construction
			// process repeats itself.
			ReadHeader();
		}

		/**
		 * @brief Scrambling the data
		 * 
		 * @param nInput 
		 * @return uint64_t 
		 */
		uint64_t scramble(uint64_t nInput)
		{
			uint64_t out = nInput ^ 0xDEABD0C123;
			out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0xF0F0F0F0F0F0F0) << 4;
			return out ^ 0xC0DFEACE12345;
		}
		
		/**
		 * @brief Asynchronous function used by both client and server to write the validation packet. This is the packet used for
		 * handshaking between the two and establishing only valid connections
		 * 
		 */
		void WriteValidation()
		{
			NET_CONNECTION_LOG_TRACE("Write validation being done for type: {0}", OWNER_TYPE_TO_STRING(m_nOwnerType))
			asio::async_write(m_socket, asio::buffer(&m_nHandshakeOut, sizeof(uint64_t)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						// Validation data sent, clients should sit and wait for a response or closure
						if(m_nOwnerType == owner::client)
						{
							NET_CONNECTION_LOG_TRACE("[CONNECTION]: CLIENT Handshake sent {0}", m_nHandshakeOut)
							NET_CONNECTION_LOG_TRACE("[CONNECTION]: CLIENT Waiting for server response...")
							ReadHeader();
						}
					}
					else
					{
						m_socket.close();
					}
				});

		}

		void ReadValidation(server_interface<T>* server = nullptr)
		{
			asio::async_read(m_socket, asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
				[this, server](std::error_code ec, std::size_t length)
				{						
					if (!ec)
					{
						if(m_nOwnerType == owner::server)
						{
							NET_CONNECTION_LOG_INFO("[CONNECTION] SERVER TODO: FILL THIS IN")
							if(m_nHandshakeIn == m_nHandshakeCheck)
							{
								NET_CONNECTION_LOG_INFO("[CONNECTION] SERVER TODO: CLIENT SUCCESS COMPLETED PUZZLE")
								server->OnClientValidated(this->shared_from_this());

								// Sit and wait to receive data now
								ReadHeader();
							}
							else
							{
								NET_CONNECTION_LOG_INFO("[CONNECTION] SERVER TODO: CLIENT FAILED TO COMPLETED PUZZLE")
								// remove the connection if they fail the test
								m_socket.close();
							}
						}
						else
						{
							NET_CONNECTION_LOG_INFO("[CONNECTION] CLIENT TODO: FILL THIS IN")
							// Clients need to transorm the input data from server
							m_nHandshakeOut = scramble(m_nHandshakeIn);
							// and send the result
							WriteValidation();
						}
					}
					else
					{
						NET_CONNECTION_LOG_ERROR("[CONNECTION] Disconnectined in ReadValidation()")
						m_socket.close();
					}
				});

		}

	protected:
		// Each connection has a unique socket to a remote 
		asio::ip::tcp::socket m_socket;

		// This context is shared with the whole asio instance
		asio::io_context &m_asioContext;

		// This queue holds all messages to be sent to the remote side
		// of this connection
		tsqueue<message<T>> m_qMessagesOut;

		// This references the incoming queue of the parent object
		tsqueue<owned_message<T>> &m_qMessagesIn;

		// Incoming messages are constructed asynchronously, so we will
		// store the part assembled message here, until it is ready
		message<T> m_msgTemporaryIn;

		// The "owner" decides how some of the connection behaves
		owner m_nOwnerType = owner::server;
		uint32_t id = 0;

		// Handshake validation
		// What the connection will be sending out
		uint64_t m_nHandshakeOut = 0;
		// What the connection has received as a result or as data to scramble
		uint64_t m_nHandshakeIn = 0;
		// Used by server to perform the comparison
		uint64_t m_nHandshakeCheck = 0;

	};
}
