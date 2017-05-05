#pragma once

namespace ven {

  class SocketUDP
    : public Socket
  {
  public:
    SocketUDP() {}

    virtual ~SocketUDP() override
    {
      close();
    }

    virtual bool open() override
    {
      if (sock_ != INVALID_SOCKET) {
        return true;
      }

      sock_ = WSASocket(
        AF_INET,
        SOCK_DGRAM,
        IPPROTO_UDP,
        0,
        0,
        WSA_FLAG_OVERLAPPED
      );

      if (sock_ == INVALID_SOCKET) {
        return false;
      }

      return true;
    }

    bool disable_udp_connreset()
    {
      // 클라 접속이 끊기면 recvfrom 에서 10054가 발생 가능하므로 SIO_UDP_CONNRESET 옵션 적용.
      // 해당 옵션 미사용시 recvfrom이 성공할때까지 재시도해야 함
      // https://support.microsoft.com/ko-kr/help/263823/winsock-recvfrom-now-returns-wsaeconnreset-instead-of-blocking-or-timing-out

      BOOL value = FALSE;
      DWORD bytes = 0;

      int_t ret = WSAIoctl(
        sock_,
        SIO_UDP_CONNRESET,
        &value,
        sizeof(value),
        NULL,
        0,
        &bytes,
        NULL,
        NULL);

      return (ret != SOCKET_ERROR);
    }

  };

}
