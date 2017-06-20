#pragma once

namespace ven {

  typedef char(ip_t)[16];
  typedef uint8_t(addr_buf_t)[(sizeof(sockaddr_in) + 16) * 2];

}
