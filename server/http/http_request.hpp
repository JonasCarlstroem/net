#pragma once
#include "http_method.hpp"
#include "http_types.hpp"
#include <sstream>
#include <utils/string>

namespace net::http {

struct request {
    method http_method;
    string path;
    string full_path;
    string http_version;
    string_map headers;
    list<char> body;
    string_map params;
    string_map query_params;
    string query_string;

    request() : http_method(method::Unknown) {}
    request(method m) : http_method(m) {}

    string get_header(const string& name) const {
        auto it = headers.find(name);
        return it != headers.end() ? it->second : "";
    }

    string get_body_as_string() const {
        return string(body.begin(), body.end());
    }

    static request parse(const string& raw) {
        request req;

        std::istringstream stream(raw);
        string line;

        if (!std::getline(stream, line) || line.empty()) {
            std::cerr << "Could not read request control data" << std::endl;
            throw std::invalid_argument("Could not read request control data.");
        }

        if (line.back() == '\r')
            line.pop_back();

        req = parse_control_data(line);
        req = parse_url(req.full_path);
        req = parse_headers(stream);

        std::ostringstream body_stream;
        body_stream << stream.rdbuf();
        req.set_body(body_stream.str());

        return req;
    }

  private:
    struct control_data {
        string method;
        string full_path;
        string version;

        std::tuple<string&&, string&&, string&&> operator()() {
            return std::make_tuple(
                std::move(method), std::move(full_path), std::move(version)
            );
        }
    };

    request& const operator=(control_data&& data) {
        this->http_method  = std::move(data.method);
        this->full_path    = std::move(data.full_path);
        this->http_version = std::move(data.version);
        return *this;
    }

    struct uri_data {
        string path;
        string query_string;
        string_map query_params;

        std::tuple<string&&, string&&, string_map&&> operator()() {
            return std::make_tuple(
                std::move(path), std::move(query_string),
                std::move(query_params)
            );
        }
    };

    request& const operator=(uri_data&& data) {
        this->path         = std::move(data.path);
        this->query_string = std::move(data.query_string);
        this->query_params = std::move(data.query_params);
        return *this;
    }

    request& const operator=(string_map&& headers) {
        this->headers = std::move(headers);
        return *this;
    }

    void set_body(string content) {
        body = list<char>(content.begin(), content.end());
    }

    void set_control_data(control_data&& data) {}

    static control_data parse_control_data(string line) {
        std::istringstream c_data(line);

        control_data c_d;
        if (!(c_data >> c_d.method >> c_d.full_path >> c_d.version)) {
            throw std::runtime_error("Error extracting control data");
        }

        return c_d;
    }

    static uri_data parse_url(string url) {
        uri_data u_d;

        auto pos = url.find('?');
        if (pos != string::npos) {
            u_d.path         = url.substr(0, pos);
            u_d.query_string = url.substr(pos + 1);

            std::istringstream qstream(u_d.query_string);
            string kv;

            while (std::getline(qstream, kv, '&')) {
                auto eq = kv.find('=');
                if (eq != string::npos) {
                    string key            = kv.substr(0, eq);
                    string val            = kv.substr(eq + 1);
                    u_d.query_params[key] = val;
                }
            }
        } else {
            u_d.path = url;
        }

        return u_d;
    }

    static string_map parse_headers(std::istringstream& stream) {
        string_map headers;
        string header_line;

        while (std::getline(stream, header_line) && header_line != "\r") {
            if (!header_line.empty() && header_line.back() == '\r') {
                header_line.pop_back();
            }

            auto colon = header_line.find(':');
            if (colon != string::npos) {
                string key   = header_line.substr(0, colon);
                string value = header_line.substr(colon + 1);

                if (!value.empty() && value.front() == ' ')
                    value.erase(0, 1);
                headers[key] = value;
            }
        }

        return headers;
    }
};

} // namespace net::http