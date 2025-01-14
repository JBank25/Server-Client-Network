#pragma once

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"

namespace olc
{
    namespace net
    {
		template<typename T>
		class connection : public std::enable_shared_from_this<connection<T>>
		{
		public:
            connection()
            {}

            virtual ~connection()
            {}

        public:
            bool ConnectToServer();
            bool Disconnect();
            bool IsConnecter() const;

        public:
            bool Send(const message<T>& msg);

		protected:
			// Each connection has a unique socket to a remote 
			asio::ip::tcp::socket m_socket;

			// This context is shared with the whole asio instance
			asio::io_context& m_asioContext;

			// This queue holds all messages to be sent to the remote side
			// of this connection
			tsqueue<message<T>> m_qMessagesOut;

			// This references the incoming queue of the parent object
			tsqueue<owned_message<T>>& m_qMessagesIn;

			// Incoming messages are constructed asynchronously, so we will
			// store the part assembled message here, until it is ready
			message<T> m_msgTemporaryIn;

			// The "owner" decides how some of the connection behaves
			owner m_nOwnerType = owner::server;

			uint32_t id = 0;
        };
    }
}