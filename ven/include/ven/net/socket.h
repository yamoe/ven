#pragma once

namespace ven {

  class Socket
  {
  protected:
    SOCKET sock_ = INVALID_SOCKET;

  public:
    Socket() {}

    virtual ~Socket() { close(); }

    virtual bool open() = 0;

    virtual void close()
    {
      if (sock_ != INVALID_SOCKET)
      {
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
      }
    }

    operator SOCKET() { return sock_; }

    operator HANDLE() { return (HANDLE)sock_; }

    operator bool() { return (sock_ != INVALID_SOCKET); }

    bool bind(const Addr& addr = Addr())
    {
      sockaddr_in saddr = addr.sockaddr();
      int32_t ret =
        ::bind(
          sock_,
          reinterpret_cast<struct sockaddr*>(&saddr),
          sizeof(saddr)
        );
      return (ret != SOCKET_ERROR);
    }

    bool set_linger()
    {
      struct linger v;
      v.l_onoff = 1;
      v.l_linger = 0;

      int32_t ret = ::setsockopt(
        sock_,
        SOL_SOCKET,
        SO_LINGER,
        reinterpret_cast<const char*>(&v),
        sizeof(v)
      );
      return (ret != SOCKET_ERROR);
    }

    bool set_send_buffer_size(uint32_t size)
    {
      int32_t ret = ::setsockopt(
        sock_,
        SOL_SOCKET,
        SO_SNDBUF,
        reinterpret_cast<const char*>(&size),
        sizeof(size)
      );
      return (ret != SOCKET_ERROR);
    }

    bool set_recv_buffer_size(uint32_t size)
    {
      int32_t ret = ::setsockopt(
        sock_,
        SOL_SOCKET,
        SO_RCVBUF,
        reinterpret_cast<const char*>(&size),
        sizeof(size)
      );
      return (ret != SOCKET_ERROR);
    }

    uint32_t send_buffer_size()
    {
      uint32_t size = 0;
      int32_t len = sizeof(size);
      int32_t ret = ::getsockopt(
        sock_,
        SOL_SOCKET,
        SO_SNDBUF,
        reinterpret_cast<char*>(&size),
        &len
      );
      return size;
    }

    uint32_t recv_buffer_size()
    {
      uint32_t size = 0;
      int32_t len = sizeof(size);
      int32_t ret = ::getsockopt(
        sock_,
        SOL_SOCKET,
        SO_RCVBUF,
        reinterpret_cast<char*>(&size),
        &len
      );
      return size;
    }

    bool shutdown_both()
    {
      int32_t ret = shutdown(sock_, SD_BOTH);
      if (ret == SOCKET_ERROR) {
        return false;
      }
      return (ret != SOCKET_ERROR);
    }

    bool set_reuse(bool b)
    {
      int v = (b) ? 1 : 0;
      int ret =
        ::setsockopt(
          sock_,
          SOL_SOCKET,
          SO_REUSEADDR,
          reinterpret_cast<const char*>(&v),
          sizeof(v)
        );
      return (ret != SOCKET_ERROR);
    }

  };

}
