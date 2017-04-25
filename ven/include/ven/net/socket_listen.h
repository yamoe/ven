#pragma once

namespace ven
{

  class SocketListen
    : public Socket
  {
  private:
    ui32_t buf_len_ = sizeof(sockaddr_in) + 16;

    LPFN_ACCEPTEX _acceptex = nullptr;
    LPFN_GETACCEPTEXSOCKADDRS _getacceptexsockaddrs = nullptr;

  public:
    SocketListen() { open(); }

    virtual ~SocketListen() override
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
        SOCK_STREAM,
        IPPROTO_TCP,
        0,
        0,
        WSA_FLAG_OVERLAPPED
      );

      if (sock_ == INVALID_SOCKET) {
        return false;
      }

      _acceptex = FuncGetter::acceptex(sock_);
      if (!_acceptex) return false;

      _getacceptexsockaddrs = FuncGetter::getacceptexsockaddrs(sock_);
      if (!_getacceptexsockaddrs) return false;

      return true;
    }

    bool acceptex(OVERLAPPED* ov, SOCKET sock, addr_buf_t& buf)
    {
      DWORD recv_bytes = 0;
      return (_acceptex(
        sock_,
        sock,
        buf,
        0,
        buf_len_,
        buf_len_,
        &recv_bytes,
        ov
      ) == TRUE);
    }

    void getacceptexsockaddrs(addr_buf_t& buf, Addr& addr)
    {
      sockaddr_in* local_addr = nullptr;
      sockaddr_in* remote_addr = nullptr;
      int_t local_len = 0;
      int_t remote_len = 0;

      _getacceptexsockaddrs(
        buf,
        0,
        buf_len_,
        buf_len_,
        reinterpret_cast<sockaddr**>(&local_addr),
        &local_len,
        reinterpret_cast<sockaddr**>(&remote_addr),
        &remote_len
      );

      addr.set_sockaddr(*remote_addr);
    }

    // AcceptEx() 호출된 경우만 accept 받기위한 설정
    bool set_conditional_accept(bool b)
    {
      int_t v = (b) ? 1 : 0;
      int_t ret =
        ::setsockopt(
          sock_,
          SOL_SOCKET,
          SO_CONDITIONAL_ACCEPT,
          reinterpret_cast<const char*>(&v),
          sizeof(v)
        );
      return (ret != SOCKET_ERROR);
    }

    bool listen(int_t backlog = 10)
    {
      return (::listen(sock_, backlog) != SOCKET_ERROR);
    }

  };

}
