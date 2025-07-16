#pragma once
#include <WinSock2.h>
#include <mutex>
#include <algorithm>
#include <socket>

namespace net {

class socket_set {
  public:
    socket_set() { FD_ZERO(&fds_); }

    void add(const socket &s) {
        std::lock_guard lock(mutex_);
        add(to_socket(s));
    }

    void add(SOCKET s) {
        std::lock_guard lock(mutex_);
        if (!contains_nolock(s)) {
            FD_SET(s, &fds_);
        }
    }

    void remove(const socket &s) {
        std::lock_guard lock(mutex_);
        SOCKET raw = to_socket(s);
        FD_CLR(raw, &fds_);
    }

    void remove(SOCKET s) {
        std::lock_guard lock(mutex_);
        FD_CLR(s, &fds_);
    }

    bool contains(SOCKET s) const {
        std::lock_guard lock(mutex_);
        return contains_nolock(s);
    }

    void clear() {
        std::lock_guard lock(mutex_);
        FD_ZERO(&fds_);
    }

    size_t size() const {
        std::lock_guard lock(mutex_);
        return fds_.fd_count;
    }

    socket_set snapshot() const {
        std::lock_guard lock(mutex_);
        return *this;
    }

    SOCKET get(int index) const {
        std::lock_guard lock(mutex_);
        return index < 0 || index > fds_.fd_count
                   ? throw std::out_of_range(
                         "Index was outside the bounds of the array."
                     )
                   : fds_.fd_array[index];
    }

    int select() {
        int result = ::select(0, &fds_, nullptr, nullptr, nullptr);
        if (result == SOCKET_ERROR) {
            net::socket_error err = net::get_socket_error();
            std::cerr << "[select] failed: " << err.message
                      << ", error code: " << err.error_code << std::endl;
        }
        return result;
    }

    fd_set &raw() { return fds_; }

    operator const fd_set &() { return fds_; }

    fd_set *operator&() { return &fds_; }

  private:
    socket_set(const fd_set &set) : fds_(set) {}

    socket_set(const socket_set &set) : fds_(set.fds_) {}

    SOCKET to_socket(const socket &s) const {
        return static_cast<SOCKET>(s);
    }

    bool contains_nolock(SOCKET s) const {
        for (u_int i = 0; i < fds_.fd_count; ++i) {
            if (fds_.fd_array[i] == s) {
                return true;
            }
        }
        return false;
    }

    mutable std::mutex mutex_;
    fd_set fds_;
};

} // namespace net