#include <iostream>
#include "Client.h"
#include <thread>

int main(int argc, char** argv)
{
	IpEndpoint ipendpoint{ "192.168.100.5", 5000 };
	Client client{ ipendpoint };
	
	while (true)
	{
		if (client.start())
		{
			break;
		}
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}
	

	std::this_thread::sleep_for(std::chrono::seconds(2000));
	return 0;
}