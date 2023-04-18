#include "Client.h"
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <atomic>
#include <unordered_map>

#include <iostream>

#define BUF_SIZE  100

struct Client::Impl 
{
	Impl(const IpEndpoint& ipEndPoint)
		:m_ipEndpoint(ipEndPoint)
	{
	}

	~Impl()
	{
		stop();
	}

	bool start()
	{
		if (m_isInitialized)
		{
			std::cout << "Client already started" << std::endl;
			return true;
		}

		WSADATA wsaData;
		int wsaret = WSAStartup(0x101, &wsaData);
		if (wsaret != 0)
		{
			std::cout << "Win sock initialization failed" << std::endl;
			return false;
		}
		m_clientSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (m_clientSocket == INVALID_SOCKET)
		{
			std::cout << "Client socket creation failed" << std::endl;
			return false;
		}

		auto serverAddr = makeSockAddr();
		int status;
		if ((status = connect(m_clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr))) < 0)
		{
			std::cout << "Server connecting failed" << std::endl;
			reset();
			return false;
		}

		std::cout << "Server connected successfully" << std::endl;
		m_isInitialized = true;

		m_canRunRead = true;
		m_readerThread = std::make_unique<std::thread>(&Impl::read, this);

		m_canRunWrite = true;
		m_writerThread = std::make_unique<std::thread>(&Impl::write, this);
		return true;
	}

	void stop()
	{
		if (!m_isInitialized)
		{
			std::cout << "Server not started" << std::endl;
			return;
		}

		reset();
		
		std::cout << "Server stopped successfully" << std::endl;
	}

	void read()
	{
		char buffer[BUF_SIZE];

		while (m_canRunRead)
		{
			memset(buffer, 0, BUF_SIZE);
			auto readBytes = recv(m_clientSocket, buffer, BUF_SIZE, 0);// MSG_WAITALL);
			if (readBytes > 0)
			{
				std::cout << "Read " << std::to_string(readBytes) << " bytes : " << std::string(buffer, readBytes) << std::endl;
			}
			else
			{
				std::cout << "Client disconnected" << std::endl;
				break;
			}
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}
	}

	void write()
	{
		char buffer[BUF_SIZE];
		std::string msg = "Client message_";
		int i = 0;

		while (m_canRunWrite)
		{
			memset(buffer, 0, BUF_SIZE);
			auto newMsg = msg + std::to_string(i++);
			strcpy_s(buffer, BUF_SIZE, newMsg.c_str());
			
			auto writtenBytes = send(m_clientSocket, buffer, newMsg.size(), 0);
			if (writtenBytes > 0)
			{
				std::cout << "Written " << std::to_string(writtenBytes) << " bytes" << std::endl;
			}
			std::this_thread::sleep_for(std::chrono::seconds(2));
		}
	}

	void reset()
	{
		if (m_readerThread || m_writerThread)
		{
			m_canRunRead = false;
			m_canRunWrite = false;

			m_readerThread->join();
			m_writerThread->join();

			m_readerThread = nullptr;
			m_writerThread = nullptr;
		}

		if (m_clientSocket != INVALID_SOCKET)
		{
			closesocket(m_clientSocket);
		}
		WSACleanup();
		m_isInitialized = false;
	}

	sockaddr_in makeSockAddr()
	{
		sockaddr_in serverAddr;
		serverAddr.sin_family = AF_INET; //Address family
		//serverAddr.sin_addr.s_addr = INADDR_ANY; //Wild card IP address
		serverAddr.sin_port = htons((u_short)m_ipEndpoint.Port); //port to use
		inet_pton(AF_INET, m_ipEndpoint.IpAddress.c_str(), &serverAddr.sin_addr);
		return serverAddr;
	}

	IpEndpoint m_ipEndpoint;
	unsigned int m_maxConnection;

	SOCKET m_clientSocket;
	std::atomic_bool m_isInitialized;

	std::unique_ptr<std::thread> m_readerThread;
	std::unique_ptr<std::thread> m_writerThread;	
	std::atomic_bool m_canRunRead;
	std::atomic_bool m_canRunWrite;
};

Client::Client(const IpEndpoint& ipEndPoint)
	:m_impl(std::make_unique<Impl>(ipEndPoint))
{
}

Client::~Client()
{
}

bool Client::start()
{
	return m_impl->start();
}

void Client::stop()
{
	m_impl->stop();
}