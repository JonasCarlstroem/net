#pragma once
#include <utils/string>
#include <utils/macros>

namespace net::http {

struct status_debug_view;

struct status {
    int code;
    string message;

    string to_string() const { return std::to_string(code) + " " + message; }

#ifdef _DEBUG
    status_debug_view debug() const;
#endif
};

#ifdef _DEBUG
struct status_debug_view {
    const int &code;
    const string &message;
    string full_text;

    status_debug_view(const status &status)
        : code(status.code), message(status.message),
          full_text(status.to_string()) {}
};
#endif

} // namespace http