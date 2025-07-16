#include "../http_status.hpp"

#ifdef _DEBUG
NOINLINE http_status_debug_view http_status::debug() const {
    return http_status_debug_view(*this);
}
#endif