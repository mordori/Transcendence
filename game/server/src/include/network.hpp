#pragma once
#include <string>

namespace server::network {

void init(int port);
void broadcast(const std::string& json);
}
