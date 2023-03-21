#pragma once

#include <cstddef>

namespace messaging {
struct Header {
  size_t body_len;
};
static constexpr auto HEADER_SIZE = sizeof(messaging::Header);

} // namespace messaging
