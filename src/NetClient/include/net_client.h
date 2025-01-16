#pragma once

#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"
#include "net_connection.h"


namespace olc
{
    namespace net
    {
        template <typename T>
        class client_interface
        {

            client_interface() : m_socket(m_context)
            {

            }

            virtual ~client_interface()
            {
                Disconnect();
            }

		public:
			// Connect to server with hostname/ip-address and port
			bool Connect(const std::string& host, const uint16_t port)
			{
                return false;
			}

			// Disconnect from server
			void Disconnect()
			{
			}

			// Check if client is actually connected to a server
			bool IsConnected()
			{
                return false;
			}

		public:
			// Retrieve queue of messages from server
			tsqueue<owned_message<T>>& Incoming()
			{ 
				return m_qMessagesIn;
			}


		protected:
			// asio context handles the data transfer...
			asio::io_context m_context;
			// ...but needs a thread of its own to execute its work commands
			std::thread thrContext;
            // This is a hardware socket that is connected to the server
            asio::ip::tcp::socket m_socket;
			// The client has a single instance of a "connection" object, which handles data transfer
			std::unique_ptr<connection<T>> m_connection;
			
		private:
			// This is the thread safe queue of incoming messages from server
			tsqueue<owned_message<T>> m_qMessagesIn;
        };
        
    } // namespace net
    
    
} // namespace olc

