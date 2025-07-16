#pragma once
#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <utils/net>
#define IS_NULL(ptr) ptr == nullptr
#define IS_IP_NULL IS_NULL(ip_ptr_)
#define IS_PORT_NULL IS_NULL(port_ptr_)
#define IS_PTR_NULL IS_NULL(ptr_)

using string = std::string;

namespace net {

template <typename TPtr> class base_ptr {
    bool is_manual = false;
    TPtr *ptr_;

  protected:
    base_ptr() : ptr_(nullptr) { manual_ptr(); }
    explicit base_ptr(TPtr *ptr) : ptr_(ptr) {}

    ~base_ptr() {
        if (is_manual) {
            delete ptr_;
        }
    }

    void manual_ptr() {
        if (IS_PTR_NULL) {
            ptr_      = new TPtr();
            is_manual = true;
        }
    }

    TPtr *get_ptr() const { return ptr_; }
};

class ip_address /* : protected base_ptr<uint32_t> */ {
    friend class ip_endpoint;

  public:
    static string from_binary(uint32_t ip) {
        struct in_addr addr;
        char _ip[INET_ADDRSTRLEN];
        return inet_ntop(AF_INET, &ip, _ip, INET_ADDRSTRLEN);
    }

    static uint32_t from_string(const string &ip) {
        struct in_addr addr;
        if (inet_pton(AF_INET, ip.c_str(), &addr) != 1) {
            std::cerr << "Invalid IP Address" << std::endl;
            throw std::invalid_argument("Invalid IP Address");
        }
        return addr.s_addr;
    }
};

class ip_port : protected base_ptr<uint16_t> {
    friend class ip_endpoint;

  public:
    ip_port() : base_ptr() {}

    explicit ip_port(uint16_t *val) : base_ptr(val) {
        if (*val > 65535)
            throw std::out_of_range("Invalid port number");
    }

    const ip_port &operator=(int port) {
        set(port);
        return *this;
    }

    const ip_port &operator=(uint16_t port) {
        set(port);
        return *this;
    }

    uint16_t value() const { return ntohs(*get_ptr()); }

    void set(uint16_t val) { *get_ptr() = htons(val); }

    uint16_t raw() { return *get_ptr(); }

    operator const int() const { return value(); }

    bool operator==(const ip_port &other) const {
        return *get_ptr() == *other.get_ptr();
    }

    operator const uint16_t &() const { return *get_ptr(); }
};

class ip_endpoint {
    friend class socket;
    // friend ip_address;
    friend ip_port;

    sockaddr_in saddr_;
    string ip_;
    int port_;

    socklen_t len{};

  public:
    ip_endpoint()
        : len(sizeof(saddr_)), saddr_{}, ip_(DEFAULT_IP), port_(DEFAULT_PORT) {
        saddr_.sin_family      = AF_INET;
        saddr_.sin_addr.s_addr = ip_address::from_string(ip_);
        saddr_.sin_port        = htons(port_);
    }

    ip_endpoint(const string &ip, int port) : saddr_{}, ip_(ip), port_(port) {
        saddr_.sin_family      = AF_INET;
        saddr_.sin_addr.s_addr = ip_address::from_string(ip_);
        saddr_.sin_port        = htons(port_);
    }

    const string ip_address() { return ip_; }
    const string &ip_address() const { return ip_; }

    const int port() { return port_; }
    const int &port() const { return port_; }

    sockaddr_in &native() { return saddr_; }
    const sockaddr_in &native() const { return saddr_; }

    string to_string() { return ip_ + ":" + std::to_string(port_); }

    void set(const string &ip, uint16_t port) {
        ip_  = ip;
        port = port;
    }

    sockaddr *get_sockaddr() { return reinterpret_cast<sockaddr *>(&saddr_); }

    const sockaddr *get_sockaddr() const {
        return reinterpret_cast<const sockaddr *>(&saddr_);
    }

    operator const sockaddr_in &() const { return saddr_; }

    sockaddr_in &get_sockaddr_in() {
        return *(reinterpret_cast<sockaddr_in *>(this));
    }

    const sockaddr_in &get_sockaddr_in() const {
        return *(reinterpret_cast<const sockaddr_in *>(this));
    }

    socklen_t *get_len_ptr() { return &len; }

    socklen_t size() const { return sizeof(saddr_); }
};

} // namespace net