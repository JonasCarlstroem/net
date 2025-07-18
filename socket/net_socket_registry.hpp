#pragma once
// std
#include <queue>
#include <types>
#include <unordered_map>
#include <unordered_set>

// lib
#include "net_socket_set.hpp"

namespace net {

template <typename T>
using uni_ptr  = std::unique_ptr<T>;

using sock_ptr = std::shared_ptr<socket>;

class socket_registry {
    std::unordered_map<SOCKET, sock_ptr> sockets_;
    socket_set socket_set_;

    std::unordered_set<SOCKET> sockets_in_progress;
    std::mutex progress_mutex;

    std::queue<socket> close_queue;
    std::mutex close_mutex;
    mutable std::mutex snapshot_mutex;

  public:
    void set(sock_ptr sock) {
        SOCKET raw = socket::to_socket(*sock);
        sockets_.emplace(raw, sock);
        socket_set_.add(raw);
    }

    void add(sock_ptr socket) {
        SOCKET raw = socket::to_socket(*socket);
        sockets_.emplace(raw, socket);
        socket_set_.add(raw);
    }

    bool contains(SOCKET s) const { return sockets_.find(s) != sockets_.end(); }

    sock_ptr get(SOCKET s) { return sockets_.at(s); }

    sock_ptr take(SOCKET s) {
        auto it = sockets_.find(s);
        if (it == sockets_.end())
            return nullptr;

        sock_ptr out = it->second;
        sockets_.erase(it);
        socket_set_.remove(s);
        return out;
    }

    sock_ptr create_socket(const string& ip = "", int port = 0) {
        sock_ptr ptr = std::make_shared<socket>(ip, port);
        SOCKET raw   = socket::to_socket(*ptr);
        sockets_.emplace(raw, ptr);
        socket_set_.add(raw);

        return ptr;
    }

    sock_ptr create_socket(SOCKET s) {
        sock_ptr ptr = std::make_shared<socket>(s);
        sockets_.emplace(s, ptr);
        socket_set_.add(s);

        return ptr;
    }

    void set_in_progress(SOCKET s) {
        std::lock_guard lock(progress_mutex);
        sockets_in_progress.insert(s);
    }

    bool is_in_progress(SOCKET s) {
        std::lock_guard lock(progress_mutex);
        return sockets_in_progress.count(s) > 0;
    }

    void remove_in_progress(SOCKET s) {
        std::lock_guard lock(progress_mutex);
        sockets_in_progress.erase(s);
    }

    void mark_closed(SOCKET s) {
        std::scoped_lock lock(snapshot_mutex, close_mutex);
        if (sockets_.find(s) != sockets_.end()) {
            close_queue.push(s);
        }
    }

    void drain_closed() {
        std::scoped_lock lock(snapshot_mutex, close_mutex);
        while (!close_queue.empty()) {
            SOCKET s = close_queue.front();
            close_queue.pop();

            auto it = sockets_.find(s);
            if (it != sockets_.end()) {
                it->second->close();
                sockets_.erase(it);
                socket_set_.remove(s);
            }
        }
    }

    socket_set snapshot() const {
        std::scoped_lock lock(snapshot_mutex);
        return socket_set_.snapshot();
    }

    void clear() {
        for (auto& [s, sock] : sockets_) {
            sock->close();
        }
        sockets_.clear();
        socket_set_.clear();
    }
};

} // namespace net