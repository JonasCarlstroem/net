#include "../http_response.hpp"

#ifdef _DEBUG
NOINLINE http_response_debug_view http_response::debug() const {
    return http_response_debug_view(*this);
}
#endif