#pragma once
// std
#include <iostream>

// lib
#include <net/socket_registry>
#include "http_request.hpp"
#include "http_response.hpp"
#include "routing/router.hpp"

namespace net::http {

class connection_handler {
    friend class server;

  public:
    connection_handler(net::sock_ptr socket, router& r) : client_socket(socket), r(r) {}

    ~connection_handler() {
        if (client_socket->is_valid())
            client_socket->close();
    }

    void handle(const string& request_text) {
        response res;
        try {
            request req = request::parse(request_text);

            if (!r.route_request(req, res)) {
                res.set_status(404, "Not Found");
            }
        } catch (const std::exception& ex) {
            res.set_status(500, "Internal server error");
        }

        client_socket->write(res.to_string());
    }

  private:
    net::sock_ptr client_socket;
    router& r;

    bool parse_http_request(const string& text, request& req) {
        std::istringstream stream(text);
        string line;

        if (!std::getline(stream, line) || line.empty())
            return false;

        std::istringstream request_line(line);
        string m;
        if (!(request_line >> m >> req.path >> req.http_version))
            return false;

        req.http_method = m;

        while (std::getline(stream, line) && !line.empty() && line != "\r") {
            if (line.back() == '\r')
                line.pop_back();

            auto colon_pos = line.find(':');
            if (colon_pos == string::npos)
                continue;

            string header_name  = line.substr(0, colon_pos);
            string header_value = line.substr(colon_pos + 1);

            while (!header_value.empty() &&
                   (header_value.front() == ' ' || header_value.front() == '\t'))
                header_value.erase(header_value.begin());

            req.headers[header_name] = header_value;
        }

        auto it = req.headers.find("Content-Length");
        if (it != req.headers.end()) {
            int content_length = std::stoi(it->second);
            if (content_length > 0) {
                list<char> body(content_length);
                stream.read(body.data(), content_length);
                req.body = std::move(body);
            }
        }

        return true;
    }
};

} // namespace net::http