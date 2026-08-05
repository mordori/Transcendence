#pragma once
#include <string>

namespace network
{
	void init(int port);
	void broadcast(const std::string &json);
}
