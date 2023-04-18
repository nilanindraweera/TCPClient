#pragma once
#include "DataTypes.h"
#include <memory>

class Client
{
public:
	explicit Client(const IpEndpoint& ipEndPoint);
	~Client();

	bool start();
	void stop();
	
private:
	struct Impl;
	std::unique_ptr<Impl> m_impl;
};