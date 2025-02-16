#pragma once

//#include "spdlog/spdlog.h"
#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_connection.h"
#include "net_log.h"

namespace net
{
	template<typename T>
	class server_interface
	{
	public:
		/**
		 * @brief Construct a new server interface object. m_asioAcceptor will be responsible for
		 * accepting incoming TCP connections. The endpoint created for our acceptor is TVP v4 
		 * on port.
		 * 
		 * @param port - port number on which the acceptor will bind
		 */
		server_interface(uint16_t port)
			: m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
		{
			log::Init();
			// spdlog::set_pattern("[%^%l%$]  %v");
		}

		virtual ~server_interface()
		{
			// May as well try and tidy up
			Stop();
		}

		// Starts the server!
		bool Start()
		{
			// spdlog::info("[SERVER]: Start()");
			try
			{
				// Issue a task to the asio context - This is important
				// as it will prime the context with "work", and stop it
				// from exiting immediately. Since this is a server, we 
				// want it primed ready to handle clients trying to
				// connect.
				WaitForClientConnection();

				/**
				 * Launch the asio context in its own thread. If we did this prior to
				 * WaitForClientConnection() then we would run the risk of the asio context running
				 * out of work immediately
				 */
				m_threadContext = std::thread([this]() { m_asioContext.run(); });
			}
			catch (std::exception& e)
			{
				// Something prohibited the server from listening
				// spdlog::info("[SERVER]: Exception {0}", e.what());
				return false;
			}

			// spdlog::info("[SERVER]: started!");
			return true;
		}

		// Stops the server!
		void Stop()
		{
			// spdlog::info("[SERVER]: Stop()");
			// Request the context to close
			m_asioContext.stop();

			// Tidy up the context thread
			if (m_threadContext.joinable()) m_threadContext.join();

			// Inform someone, anybody, if they care...
			// spdlog::info("[SERVER]: Stopped");
		}

		// ASYNC - Instruct asio to wait for connection
		void WaitForClientConnection()
		{
			// Prime context with an instruction to wait until a socket connects. This
			// is the purpose of an "acceptor" object. It will provide a unique socket
			// for each incoming connection attempt
			// spdlog::info("[SERVER]: WaitForClientConnection()");
			m_asioAcceptor.async_accept(
				[this](std::error_code ec, asio::ip::tcp::socket socket)
				{
					// Triggered by incoming connection request
					if (!ec)
					{
						// Display some useful(?) information
						std::string address = socket.remote_endpoint().address().to_string();
						unsigned short port = socket.remote_endpoint().port();
						// spdlog::info("[SERVER]: New Connection: {0} {1}", address, port);

						/**
						 * Create new connection to handle the client. Here we are telling the 
						 * connection it is owned by a server. This is so we can tailor how a connection
						 * behaves depending on if it is owned by a client or server. 
						 * 
						 * The server's incoming message queue is also passed by reference to the new connection
						 * ALL connections to the server will share this.
						 * 
						 * If server rejects the connection then it will go out of scope and be erased from
						 * memory (motivating why we used shared_ptr's here)
						 */
						std::shared_ptr<connection<T>> newconn = 
							std::make_shared<connection<T>>(connection<T>::owner::server, 
								m_asioContext, std::move(socket), m_qMessagesIn);
						
						

						// Give the user server a chance to deny connection
						if (OnClientConnect(newconn))
						{								
							// Connection allowed, so add to container of new connections
							m_deqConnections.push_back(std::move(newconn));

							// And very important! Issue a task to the connection's
							// asio context to sit and wait for bytes to arrive!
							m_deqConnections.back()->ConnectToClient(nIDCounter++);

							// spdlog::info("[SERVER]: Connection Approved - [{0}]", m_deqConnections.back()->GetID());
						}
						else
						{
							// spdlog::info("[SERVER]: Connection Denied");

							// Connection will go out of scope with no pending tasks, so will
							// get destroyed automagically due to the wonder of smart pointers
						}
					}
					else
					{
						// Error has occurred during acceptance
						// spdlog::error("[SERVER]: New Connection Error: {0}", ec.message());
					}

					// Prime the asio context with more work - again simply wait for
					// another connection...
					WaitForClientConnection();
				});
		}

		/**
		 * @brief - Handles messaging to a specific client. If the client is still valid this is simply
		 * a wrapper to the Send() function. If the client is not valid then we can assmume that communication
		 * has stopped and handle the clean removal of this connection.
		 * 
		 * @param client - client to be messaged
		 * @param msg - message to send
		 */
		void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
		{
			// Check client is valid and that it is still connection
			if (client && client->IsConnected())
			{
				// post the message via the connection
				client->Send(msg);
			}
			else
			{
				// If we cant communicate with client then we may as 
				// well remove the client - let the server know, it may
				// be tracking it somehow
				OnClientDisconnect(client);

				// Off you go now, bye bye!
				client.reset();

				// Then physically remove it from the container
				m_deqConnections.erase(
					std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end());
			}
		}
		
		/**
		 * @brief - Handles messaging to all clients. Will iterate over all clients in connections deque.
		 * If the client is still valid this is simply a wrapper to the Send() function. 
		 * If the client is not valid then we can assmume that communication
		 * has stopped and handle the clean removal of this connection. Removal could be costly
		 * in terms of performance AND we don't want to invalidate our iterator, thus we use a flag
		 * to check if any invalid connections exist THEN we remove them
		 * 
		 * @param msg - message to send to all clients
		 * @param pIgnoreClient - client to skip messaging, bull by default
		 */
		void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr)
		{
			bool bInvalidClientExists = false;

			// Iterate through all clients in container
			for (auto& client : m_deqConnections)
			{
				// Check client is connected...
				if (client && client->IsConnected())
				{
					// ..it is!
					if(client != pIgnoreClient)
						client->Send(msg);
				}
				else
				{
					// The client couldnt be contacted, so assume it has
					// disconnected.
					OnClientDisconnect(client);
					client.reset();

					// Set this flag to then remove dead clients from container
					bInvalidClientExists = true;
				}
			}

			// Remove dead clients, all in one go - this way, we dont invalidate the
			// container as we iterated through it.
			if (bInvalidClientExists)
				m_deqConnections.erase(
					std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
		}

		/**
		 * @brief - Called by the user to explicitly process some messages in the queue. 
		 * Allows user to decide when to handle incoming messages in server side application
		 * logic. 
		 * 
		 * @param nMaxMessages - number of messages to process, defaults to max
		 * @param bWait 
		 */
		void Update(size_t nMaxMessages = -1, bool bWait = false)
		{
			// call to blocking wait() function in tsqueue
			if (bWait) m_qMessagesIn.wait();

			// Process as many messages as you can up to the value
			// specified
			size_t nMessageCount = 0;
			while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
			{
				// Grab the front message
				auto msg = m_qMessagesIn.pop_front();

				// Pass to message handler
				OnMessage(msg.remote, msg.msg);

				nMessageCount++;
			}
		}

	protected:
		// This server class should override thse functions to implement
		// customised functionality

		// Called when a client connects, you can veto the connection by returning false
		virtual bool OnClientConnect(std::shared_ptr<connection<T>> client)
		{
			return false;
		}

		// Called when a client appears to have disconnected
		virtual void OnClientDisconnect(std::shared_ptr<connection<T>> client)
		{

		}

		// Called when a message arrives
		virtual void OnMessage(std::shared_ptr<connection<T>> client, message<T>& msg)
		{

		}


	protected:
		// Thread Safe Queue for incoming message packets
		tsqueue<owned_message<T>> m_qMessagesIn;

		/**
		 * std::deque containing shared ptr's to all connection objects.
		 */
		std::deque<std::shared_ptr<connection<T>>> m_deqConnections;

		// Order of declaration is important - it is also the order of initialisation
		/**
		 * the io_context is the link to our OS's I/O services. Requests for I/O services from
		 * our clients will ultimately be forwarded to the I/O context for the OS to perform
		 * operations on. The io_context will translate any errors resulting from opertaions into
		 * an system::error_code object and forwarded up to the I/O object. 
		 */
		asio::io_context m_asioContext;
		std::thread m_threadContext;

		/**
		 * This class forms foundation of server. It will manage incoming
		 * connections though a three-phase lifecyle
		 * 		1. Binding to local endpoint
		 * 		2. Entering listening state
		 * 		3. Accepting incoming connections
		 */
		asio::ip::tcp::acceptor m_asioAcceptor; // Handles new incoming connection attempts...

		// Clients will be identified in the "wider system" via an ID
		uint32_t nIDCounter = 10000;
	};
}