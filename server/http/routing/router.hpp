#pragma once
#include <utils/string>
#include "../http_request.hpp"
#include "../http_response.hpp"
#include "route.hpp"

namespace net::http {

class router {
  public:
    bool route_request(request& req, response& res) {
        for (const auto& entry : routes) {
            if (!entry.method.equals(req.http_method))
                continue;
            string_map params;

            if (entry.pattern.match(req.path, params)) {
                req.params = std::move(params);
                res        = entry.handler(req);
                return true;
            }
        }
        return false;
    }

    void register_route(method method, const std::string& path, route_handler handler) {
        routes.push_back({route_pattern::from_string(path), method, handler});
    }

  private:
    list<route> routes;
    route_map get_routes;
    route_map post_routes;
};

} // namespace net::http