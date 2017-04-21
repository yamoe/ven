#pragma once

namespace ven {

  class SocketTCP
    : public Socket
  {
    bool registered_iocp_ = false;

    LPFN_CONNECTEX _connectex = nullptr;
    LPFN_DISCONNECTEX _disconnectex = nullptr;

  public:
    SocketTCP() { open(); }

    virtual ~SocketTCP() override
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

      _connectex = FuncGetter::connectex(sock_);
      if (!_connectex) return false;

      _disconnectex = FuncGetter::disconnectex(sock_);
      if (!_disconnectex) return false;

      return true;
    }

    virtual void close() override
    {
      if (sock_ != INVALID_SOCKET)
      {
        Socket::close();
        registered_iocp_ = false;
      }
    }

    bool disconnectex(OVERLAPPED* ov)
    {
      //return TransmitFile(sock_, NULL, 0, 0, ov, NULL, TF_DISCONNECT | TF_REUSE_SOCKET);
      return (_disconnectex(
        sock_,
        ov,
        TF_REUSE_SOCKET,
        0
      ) == TRUE);
    }

    bool connectex(OVERLAPPED* ov, const Addr& addr)
    {
      sockaddr_in saddr = addr.sockaddr();

      return (_connectex(
        sock_,
        reinterpret_cast<const sockaddr*>(&saddr),
        sizeof(saddr),
        0,
        0,
        0,
        ov
      ) == TRUE);
    }

    bool registered_iocp()
    {
      return registered_iocp_;
    }

    void set_registered_iocp(bool v)
    {
      registered_iocp_ = v;
    }

    bool update_accept_context(SOCKET listen_socket)
    {
      int ret = ::setsockopt(
        sock_,
        SOL_SOCKET,
        SO_UPDATE_ACCEPT_CONTEXT,
        reinterpret_cast<const char_t*>(&listen_socket),
        sizeof(listen_socket)
      );
      return (ret != SOCKET_ERROR);
    }

    bool update_connect_context()
    {
      int ret = ::setsockopt(
        sock_,
        SOL_SOCKET,
        SO_UPDATE_CONNECT_CONTEXT,
        0,
        0
      );
      return (ret != SOCKET_ERROR);
    }

  };

}
