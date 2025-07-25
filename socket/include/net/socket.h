#pragma once
// std
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>

// libs
#include "endpoint.h"

using string = std::string;

namespace net {

string make_ip(uint32_t bin) { return ip_address::from_binary(bin); }
uint32_t make_ip(string str) { return ip_address::from_string(str); }
static std::pair<string, uint32_t> any_addr =
    std::make_pair(make_ip(INADDR_ANY), make_ip(make_ip(INADDR_ANY)));

enum class protocol : int { TCP = IPPROTO_TCP, UDP = IPPROTO_UDP };

enum class address_family : unsigned short { IPv4 = AF_INET, IPv6 = AF_INET6 };

class socket {
  public:
    bool non_blocking = false;

    socket() : socket(protocol::TCP) {}

    socket(int port) : socket("", port) {}

    socket(const string& ip, int port) : socket(protocol::TCP, ip, port) {}

    socket(protocol protocol, const string& ip = "", int port = 3154)
        : _ep(ip.empty() ? any_addr.first : ip, port), _socket(INVALID_SOCKET) {
        pre_init();

        int type = protocol == protocol::TCP ? SOCK_STREAM : SOCK_DGRAM;

        _socket  = ::socket(AF_INET, type, (int)protocol);

        if (_socket == INVALID_SOCKET)
            throw std::runtime_error("Socket creation failed.");
    }

    socket(const socket& s)
        : non_blocking(s.non_blocking), _socket(s._socket), _ep(s._ep), bound(s.bound) {}

    socket& operator=(const socket& s) {
        non_blocking = s.non_blocking;
        _socket      = s._socket;
        _ep          = s._ep;
        bound        = s.bound;
        return *this;
    }

    socket(socket&& other) noexcept : _socket(other._socket), _ep(std::move(other._ep)) {
        other._socket = INVALID_SOCKET;
    }

    socket& operator=(socket&& other) noexcept {
        if (this != &other) {
            ::closesocket(_socket);
            _socket       = other._socket;
            other._socket = INVALID_SOCKET;

            _ep           = std::move(other._ep);
        }

        return *this;
    }

    bool operator==(const SOCKET& rhs) { return this->_socket == rhs; }

    bool operator==(const socket& rhs) { return this->_socket == rhs._socket; }

    ~socket() {}

    bool bind() {
        if (bound)
            return true;

        return ::bind(_socket, _ep.get_sockaddr(), _ep.size()) != SOCKET_ERROR;
    }

    bool listen(const string& ip, int port) {
        if (!ip.empty())
            _ep.ip_ = ip;

        return listen(port);
    }

    bool listen(int port) {
        if (port > 0)
            _ep.port_ = port;

        return listen();
    }

    bool listen() {
        if (!bound) {
            bound = bind();
        }

        bool is_listening = false;
        if (bound) {
            is_listening = ::listen(_socket, SOMAXCONN) != SOCKET_ERROR;

            if (is_listening && non_blocking) {
                u_long mode = 1;
                ioctlsocket(_socket, FIONBIO, &mode);
            }
        }

        return is_listening;
    }

    socket accept() {
        ip_endpoint ep;
        socket client = ::accept(_socket, ep.get_sockaddr(), ep.get_len_ptr());
        if (client == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << net::get_socket_error() << std::endl;
            return socket();
        }

        return client;
    }

    void write(const string& message) {
        int sent = ::send(_socket, message.c_str(), static_cast<int>(message.size()), 0);
        std::cout << "Sent " << sent << " bytes to the socket" << std::endl;
    }

    string read_string() {
        constexpr size_t buffer_size = 8192;
        char buffer[buffer_size];
        int bytes_read = ::recv(_socket, buffer, buffer_size, 0);

        if (bytes_read <= 0)
            return "";

        return string(buffer);
    }

    int read(char* str) {
        constexpr size_t buffer_size = 8192;
        char buffer[buffer_size];
        int bytes_read = ::recv(_socket, buffer, buffer_size, 0);

        str            = buffer;
        return bytes_read;
    }

    int read(string& out) {
        constexpr size_t buffer_size = 8192;
        char buffer[buffer_size];
        int bytes_read = ::recv(_socket, buffer, buffer_size, 0);

        if (bytes_read < 1)
            return bytes_read;

        out.append(buffer, bytes_read);
        return bytes_read;
    }

    void close() {
        ::closesocket(_socket);
        _socket = INVALID_SOCKET;
    }

    bool is_valid() const { return _socket != INVALID_SOCKET; }

    const string ip() const { return _ep.ip_; }

    const int port() const { return _ep.port_; }

    string host() const {
        string ip = _ep.ip_;
        int port  = _ep.port_;
        return ip + ":" + std::to_string(port);
    }

    operator const SOCKET&() const { return _socket; }

    socket(SOCKET sock) : _socket(sock), _ep() {
        pre_init();
        string local;
        string remote;
        bool local_error  = false;
        bool remote_error = false;

        sockaddr_in local_addr;
        int local_len = sizeof(local_addr);
        if (getsockname(_socket, (sockaddr*)&local_addr, &local_len) == 0) {
            char local_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(local_addr.sin_addr), local_ip, INET_ADDRSTRLEN);

            local = local_ip;
        } else {
            std::cerr << "getsockname failed: " << net::get_socket_error() << std::endl;
            local_error = true;
        }

        sockaddr_in remote_addr;
        int remote_len = sizeof(remote_addr);
        if (getpeername(_socket, (sockaddr*)&remote_addr, &remote_len) == 0) {
            char remote_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(remote_addr.sin_addr), remote_ip, INET_ADDRSTRLEN);

            remote = remote_ip;
        } else {
            std::cerr << "getpeername failed: " << net::get_socket_error() << std::endl;
            remote_error = true;
        }

        _ep.set(remote, remote_addr.sin_port);
    }

    const static SOCKET const to_socket(const socket& s) { return static_cast<SOCKET>(s); }

  private:
    SOCKET _socket;
    ip_endpoint _ep;
    bool bound = false;

    inline static void pre_init() {
        std::lock_guard<std::mutex> lock(ref_mutex_);
        if (ref_count_++ == 0) {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
                throw std::runtime_error("WSAStartup failed.");
            }
        }
    }

    inline static void cleanup_winsock() {
        std::lock_guard<std::mutex> lock(ref_mutex_);
        if (--ref_count_ == 0) {
            WSACleanup();
        }
    }

    inline static int ref_count_        = 0;
    inline static std::mutex ref_mutex_ = {};
};

} // namespace net
