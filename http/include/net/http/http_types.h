#pragma once
#include <string>
#include <types.h>
#define PTR_STYLE

namespace net::http {

struct request;
class response;

#ifdef PTR_STYLE
using route_handler = response (*)(const request&);
#else
using route_handler = std::function<void(const req&, res&)>;
#endif

using route_map    = dictionary<string, route_handler>;
using route_params = std::unordered_map<string, string>;

} // namespace net::http