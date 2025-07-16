#pragma once
#include "http_connection_handler.hpp"
#include <atomic>
#include <mutex>
#include <net/socket>
#include <net/socket_registry>
#include <stdexcept>
#include <thread>
#include <threading/thread_pool>
#include <unordered_map>
#include <unordered_set>

namespace net::http {

struct connection_state {
    net::sock_ptr socket;
    string buffer;

    bool read_more() {
        char temp[8192];
        while (true) {
            int result = socket->read(buffer);
            if (result > 0)
                continue;
            else if (result == 0) {
                return false;
            } else {
                int err = WSAGetLastError();
                int wb  = WSAEWOULDBLOCK;
                if (err == WSAEWOULDBLOCK)
                    return true;
                return false;
            }
        }
    }

    bool is_request_complete() const {
        return buffer.find("\r\n\r\n") != string::npos;
    }

    string take_request() {
        string req = std::move(buffer);
        buffer.clear();
        return req;
    }
};

class server {
    friend connection_handler;
    inline static constexpr const char* _default_ip = "0.0.0.0";
    inline static constexpr int _default_port       = 8080;

  public:
    bool& const non_blocking() { return _server_socket->non_blocking; }

    server() : server(_default_port) {}
    server(int port) : server(_default_ip, port) {}
    server(const string& ip, int port) : ip_(ip), port_(port), router_() {
        _server_socket = sock_registry.create_socket(ip, port);
    }

    ~server() { stop(); }

    void start() { start(ip_, port_); }

    void start(int port) { start(ip_, port); }

    void start(const string& ip, int port) {
        if (!_server_socket->listen(ip, port)) {
            std::cerr << "Failed to listen on socket "
                      << net::get_socket_error() << std::endl;
            exit(1);
        }

        run();
    }

    void stop() {
        running_ = false;
        if (_server_socket->is_valid()) {
            _server_socket->close();
            std::cout << "Server socket closed";
        }

        if (accept_thread_.joinable()) {
            accept_thread_.join();
        }
    }

    void wait() {
        std::unique_lock<std::mutex> lock(shutdown_mutex_);
        shutdown_cv_.wait(lock, [this] { return !running_.load(); });
    }

    server& get(const string path, route_handler handler) {
        router_.register_route(method::Get, path, handler);
        return *this;
    }

    server& post(const string path, route_handler handler) {
        router_.register_route(method::Post, path, handler);
        return *this;
    }

    server& put(const string path, route_handler handler) {
        router_.register_route(method::Put, path, handler);
        return *this;
    }

    server& del(const string path, route_handler handler) {
        router_.register_route(method::Delete, path, handler);
        return *this;
    }

    server& set_ip_and_port(const string& ip, int port) {
        ip_   = ip;
        port_ = port;
        return *this;
    }

  private:
    net::sock_ptr _server_socket;
    std::atomic<bool> running_;
    std::thread accept_thread_;
    string ip_;
    int port_;
    router router_;

    threading::thread_pool pool_;

    std::unordered_map<SOCKET, connection_state> conn_state;
    std::mutex conn_state_mutex;

    net::socket_registry sock_registry;
    std::mutex fd_mutex_;

    std::atomic<bool> verbose_;
    std::mutex shutdown_mutex_;
    std::condition_variable shutdown_cv_;

    void accept_connections_alt() {
        int debug_counter = 0;
        while (running_.load()) {
            sock_registry.drain_closed();
            net::socket_set read_set = sock_registry.snapshot();

            if (!read_set.select())
                break;

            for (size_t i = 0; i < read_set.size(); ++i) {
                SOCKET s = read_set.get(i);

                if (s == *_server_socket) {
                    net::sock_ptr client =
                        sock_registry.create_socket(_server_socket->accept());
                    if (client && client->is_valid()) {
                        if (non_blocking()) {
                            client->non_blocking = true;
                            u_long mode          = 1;
                            ioctlsocket(*client, FIONBIO, &mode);
                        }

                        sock_registry.add(std::move(client));
                    }
                } else {
                    if (sock_registry.is_in_progress(s))
                        continue;
                    sock_registry.set_in_progress(s);

                    net::sock_ptr client = sock_registry.get(s);

                    if (client && client->is_valid()) {
                        auto& state  = conn_state[s];
                        state.socket = client;
                        if (!state.read_more()) {
                            continue;
                        }

                        if (state.is_request_complete()) {
                            string request_data = state.take_request();

                            pool_.enqueue(
                                [this, s, client,
                                 data = std::move(request_data)]() mutable {
                                    connection_handler handler(
                                        client, router_
                                    );
                                    handler.handle(data);

                                    sock_registry.mark_closed(s);
                                    sock_registry.remove_in_progress(s);
                                }
                            );
                        }
                    }
                }
            }
        }
    }

    void run() {
        if (running_)
            return;

        running_ = true;
        std::cout << "Server running on host: " << _server_socket->host()
                  << std::endl;
        accept_thread_ =
            std::thread(&server::accept_connections_alt, this);
    }

    void cleanup(SOCKET s) {
        sock_registry.mark_closed(s);
        std::lock_guard lock(conn_state_mutex);
        conn_state.erase(s);
        sock_registry.remove_in_progress(s);
    }
};

} // namespace http