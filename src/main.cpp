#include <iostream>
#include <net_utils.h>
#include <net_server.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
	TextMessage,
};



class CustomServer : public net::server_interface<CustomMsgTypes>
{
public:
	CustomServer(uint16_t nPort) : net::server_interface<CustomMsgTypes>(nPort)
	{

	}

protected:
	virtual bool OnClientConnect(std::shared_ptr<net::connection<CustomMsgTypes>> client)
	{
		net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;
		client->Send(msg);
		return true;
	}

	// Called when a client appears to have disconnected
	virtual void OnClientDisconnect(std::shared_ptr<net::connection<CustomMsgTypes>> client)
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}

	// Called when a message arrives
	virtual void OnMessage(std::shared_ptr<net::connection<CustomMsgTypes>> client, net::message<CustomMsgTypes>& msg)
	{
		switch (msg.header.id)
		{
		case CustomMsgTypes::ServerPing:
		{
			std::cout << "[" << client->GetID() << "]: Server Ping\n";

			// Simply bounce message back to client
			client->Send(msg);
		}
		break;

		case CustomMsgTypes::MessageAll:
		{
			std::cout << "[" << client->GetID() << "]: Message All\n";

			// Construct a new message and send it to all clients
			net::message<CustomMsgTypes> msg;
			msg.header.id = CustomMsgTypes::ServerMessage;
			msg << client->GetID();
			MessageAllClients(msg, client);

		}
		break;
		case CustomMsgTypes::TextMessage:
		{
			std::cout << "[" << client->GetID() << "]: Text Message \n";
			std::cout << "Message Metadata: " << msg << std::endl;
			std::string receievedMsg;
			msg >> receievedMsg;
			std::cout << "Message: " << receievedMsg << std::endl;
		}
		break;
		}
	}
};

int main()
{
	CustomServer server(60000); 
	server.Start();

    // std::thread t([]() {
        // std::this_thread::sleep_for(std::chrono::milliseconds(250));
        // delayedFunction();
    // });

	while (1)
	{
		server.Update(-1, true);
	}
	// std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	// server.Stop();

	return 0;
}