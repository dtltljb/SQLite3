#ifndef SOCKET_JSON_PARE_HPP
#define SOCKET_JSON_PARE_HPP

#include <iostream>
#include <fstream>
#include <iterator>
#include <map>
#include <algorithm>

//third libs
#include "spline.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "nlohmann/json.hpp"
#include "blockingconcurrentqueue.h"
//user header
#include "src/socket_libevent.h"

int socket_json_pare(std::string  & json_string, struct bufferevent * dev_socket);

#endif // SOCKET_JSON_PARE_HPP
