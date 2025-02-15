#include <iostream>
#include <net_common.h>
#include <string>
#include "net_client.h"

// Get keyboard input
#include <curses.h>

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
	TextMessage,
};



class CustomClient : public net::client_interface<CustomMsgTypes>
{
public:
	void PingServer()	
	{
		net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerPing;

		// Caution with this...
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();		

		msg << timeNow;
		Send(msg);
	}

	void MessageAll()
	{
		net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::MessageAll;		
		Send(msg);
	}

	void TextServer(const std::string textMessage)	
	{
		net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::TextMessage;

		msg << textMessage;
		Send(msg);
	}
};

static std::string getAnswer()
{    
    std::string answer;
    std::cin >> answer;
    return answer;
}

int main()
{
	CustomClient c;
	c.Connect("127.0.0.1", 60000);

	bool key[3] = { false, false, false };
	bool old_key[3] = { false, false, false };

	// More keyboard input stuff
	WINDOW *win = initscr();
	nodelay(win, TRUE);
	int ch;


	bool bQuit = false;
	while (!bQuit)
	{
		ch = getch();
		switch(tolower(ch))
		{
			case 'p':
				c.PingServer();
				std::cout << "\n" << std::endl;
			break;
			case 'm':
				c.MessageAll();
				std::cout << "\n" << std::endl;
			break;
			case 't':
				c.TextServer("Hey what sup");
				std::cout << "\n" << std::endl;
			break;
			case 'q':
				bQuit = true;
				std::cout << "\n" << std::endl;
			break;
			default:
			break;
		}
		// std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		// c.PingServer();
		// std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		// c.MessageAll();


		// Actions to take depending on key press
		if (key[0] && !old_key[0]) c.PingServer();
		if (key[1] && !old_key[1]) c.MessageAll();
		if (key[2] && !old_key[2]) bQuit = true;

		for (int i = 0; i < 3; i++) old_key[i] = key[i];

		if (c.IsConnected())
		{
			if (!c.Incoming().empty())
			{
				auto msg = c.Incoming().pop_front().msg;

				switch (msg.header.id)
				{
				case CustomMsgTypes::ServerAccept:
				{
					// Server has responded to a ping request				
					std::cout << "\rServer Accepted Connection\n";
				}
				break;

				case CustomMsgTypes::ServerPing:
				{
					// Server has responded to a ping request
					std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					msg >> timeThen;
					std::cout << "\rPing: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
				}
				break;

				case CustomMsgTypes::ServerMessage:
				{
					// Server has responded to a ping request	
					uint32_t clientID;
					msg >> clientID;
					std::cout << "\rHello from [" << clientID << "]\n";
				}
				break;
				}
			}
		}
		else
		{
			std::cout << "Server Down\n";
			bQuit = true;
		}
	}
	return 0;
}