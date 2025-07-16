#pragma once
#include <sstream>
#include <utils/string>
#include <types>
#include "http_status.hpp"

using json = nlohmann::json;

namespace net::http {

#ifdef _DEBUG
struct response_debug_view;
#endif

class response {
    friend class connection_handler;
#ifdef _DEBUG
    friend response_debug_view;
#endif

  private:
    string version_ = "HTTP/1.1";
    status status_;
    string_map headers_;
    list<char> body_;

    string content_type_;

    static string const get_status_text(int code) {
        switch (code) {
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 204:
            return "No Content";
        case 400:
            return "Bad Request";
        case 404:
            return "Not Found";
        case 409:
            return "Conflict";
        case 500:
            return "Internal Server Error";
        default:
            return "Unknown";
        }
    }

    list<char> &body() { return body_; }

    string &content_type() { return content_type_; }

    void content_type(string &ct) { headers_["Content-Type"] = ct; }

    string &content_length() { return headers_["Content-Length"]; }

  public:
    response() : status_{0, ""} {}

    void set_status(int code, const string &str) {
        status_ = {code, str};
        set_body(str);
    }

    void set_status_code(int code) { status_.code = code; }
    void set_status_message(const string &str) { status_.message = str; }
    void set_header(const string &key, const string &value) {
        headers_[key] = value;
    }
    void set_headers(
        const list<std::pair<const string &, const string &>> &headers
    ) {
        for (auto &[name, value] : headers) {
            set_header(name, value);
        }
    }

    string get_body() const { return string(body_.begin(), body_.end()); }

    void set_body(const list<char> &content) { body() = content; }

    void set_body(const string &content) {
        body().assign(content.begin(), content.end());
    }

    bool is_complete() const { return status_.code != 0; }

    void set_text(const string &content) {
        body().assign(content.begin(), content.end());

        content_type_ = "text/plain";
    }

    void set_json(const json &obj) {
        string str    = obj.dump();
        body()        = list<char>(str.begin(), str.end());
        content_type_ = "application/json";
    }

    void set_binary(
        const list<char> &data, const string &c_type = "applcation/octet-stream"
    ) {
        body()        = data;
        content_type_ = c_type;
    }

    void set_html(const string &html) {
        body()        = list<char>(html.begin(), html.end());
        content_type_ = "text/html; charset=utf-8";
    }

    string to_string() const {
        std::ostringstream res;
        res << version_ << " " << status_.code << " "
            << get_status_text(status_.code) << "\r\n";
        for (const auto &[key, value] : headers_) {
            res << key << ": " << value << "\r\n";
        }

        if (!headers_.count("Content-Type") && !content_type_.empty())
            res << "Content-Type: " << content_type_ << "\r\n";
        if (!headers_.count("Content-Length"))
            res << "Content-Length: " << body_.size() << "\r\n";

        res << "Connection: close\r\n\r\n" << get_body();

        return res.str();
    }

    static response not_found(const string &message = "") {
        response res;
        res.set_status(404, message.empty() ? get_status_text(404) : message);
        return res;
    }

    static response bad_request(const string &message = "") {
        response res;
        res.set_status(400, get_status_text(400));
        return res;
    }

    static response error(const string &message = "") {
        response res;
        res.set_status(500, message.empty() ? get_status_text(500) : message);
        return res;
    }

    static response ok(const string &message = "") {
        response res;
        res.set_status(200, !message.empty() ? message : "Success");
        res.set_header("Content-Type", "text/plain");
        return res;
    }

    static response json(const json &json) {
        response res;
        res.set_status_code(200);
        res.set_json(json);
        return res;
    }

    static response html(const string &html) {
        response res;
        res.set_status_code(200);
        res.set_html(html);
        return res;
    }

#ifdef _DEBUG
    response_debug_view debug() const;
#endif
};

#ifdef _DEBUG
struct response_debug_view {
    const string &version;
    int status_code;
    const string &status_message;
    string_map headers;
    const string &body;
    string full_text;

    response_debug_view(const response &r)
        : version(r.version_), status_code(r.status_.code),
          status_message(r.status_.message), headers(r.headers_),
          body(r.get_body()), full_text(r.to_string()) {}
};
#endif

} // namespace http