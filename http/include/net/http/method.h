#pragma once
// std
#include <algorithm>
#include <map>

// lib
#include <utils/string.h>
#include "http_types.h"

namespace net::http {

struct method {
  private:
    string _method;

    static std::map<string, method> _internal;

    void set_method(const string& method) {
        if (_internal.count(method))
            _method = method;
    }

  public:
    method() : _method() {}
    method(const string& method) : _method(method) {}

    string str() const { return _method; }

    const bool equals(const method& other) const { return str().compare(other.str()) == 0; }

    static method Get;     // = method("GET");
    static method Post;    // = method("POST");
    static method Put;     // = method("PUT");
    static method Delete;  // = method("DELETE");
    static method Unknown; // = method("Unknown");

    static method parse(const string& method) {
        string formatted = trim(method);
        std::transform(formatted.begin(), formatted.end(), formatted.begin(), ::toupper);

        if (_internal.count(formatted) > 0)
            return _internal[formatted];

        return method::Unknown;
    }

    friend const std::istringstream& operator>>(std::istringstream& stream, method& method) {
        string str_method;

        stream >> str_method;
        method.set_method(str_method);

        return stream;
    }
};

inline method method::Get                  = method("GET");
inline method method::Post                 = method("POST");
inline method method::Put                  = method("PUT");
inline method method::Delete               = method("DELETE");
inline method method::Unknown              = method("UNKNOWN");

std::map<string, method> method::_internal = {{"GET", method::Get},
                                              {"POST", method::Post},
                                              {"PUT", method::Put},
                                              {"DELETE", method::Delete}};

} // namespace net::http