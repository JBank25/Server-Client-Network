#pragma once

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_connection.h"


namespace olc
{
    namespace net
    {
        template<typename T>
        class server_interface
        {
            public:
                server_interface(uint16_t listenPortNumber)
                {

                }

                virtual ~server_interface()
                {
                    StopServer();
                }

                bool StartServer()
                {
                    try
                    {
                        WaitForClientConnection();

                        m_threadContext = std::thread([this]() {m_asioContext.run(); });
                    }
                    catch(const std::exception& e)
                    {
                        // Something prohibited server from listening
                        std::cerr << "[SERVER] Exception: " << e.what() << '\n';
                    }

                    std::cout << "[SERVER] Started!\n";
                }

                bool StopServer()
                {
                    m_asioContext.stop();

                    if (m_threadContext.joinable()) m_threadContext.join();

                    std::cout << "[SERVER] Started!\n";

                }

                // ASYNC - Instruct asio to wait for connection
                void WaitForClientConnection()
                {

                }

                void MessageClient(std::shared_ptr<connection<T>> client, const message<T> &msg)
                {

                }
                void MessageAllClients(const message<T> &msg, std::shared_ptr<connection<T>> pIgnoreClient=nullptr)
                {

                }

                protected:
                    // Cakked when client connects, you can veto connection by returning false
                    virtual bool OnClientConnect(std::shared_ptr<connection<T>> client)
                    {
                        return false;
                    }

                    // Called when a client appears to have disconnected
                    virtual void OnClientDisconnect(std::shared_ptr<connection<T>> client)
                    {

                    }

                    virtual void OnMessage(std::shared_ptr<connection<T>> client, message<T> &msg)
                    {

                    }

                protected:
                    // thread safe queue for incomming message packets
                    tsqueue<owned_message<T>> m_qMessagesIn;

                    asio::io_context m_asioContext;
                    asio::thread m_threadContext;
        
                    asio::ip::tcp::acceptor m_asioAcceptor;

                    // Clients will be identified in the "wider system" via an ID
                    uint32_t nIDCounter = 10000;
        };
    }
}