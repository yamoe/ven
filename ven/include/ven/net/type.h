#pragma once

namespace ven {

  typedef char(ip_t)[16];
  typedef unsigned short port_t;
  typedef byte_t(addr_buf_t)[(sizeof(sockaddr_in) + 16) * 2];

}
