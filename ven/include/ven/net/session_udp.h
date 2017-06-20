#pragma once

namespace ven {

  class SessionUDP
    : public ISessionUDP
    , public Actor
    , public NetErrorHandler
    , public UserData
  {
  private:
    friend class Net;

    SocketUDP sock_;

    ROV rov_;
    RBuf rbuf_;

    Addr bind_addr_;
    Addr connect_addr_;
    sockaddr_in client_addr_;
    int32_t client_addr_size_ = sizeof(client_addr_);

    IOCP* iocp_ = nullptr;
    IMemPool* mpool_ = nullptr;

    std::atomic<bool> is_receiving_ = false;
    std::atomic<bool> force_close_ = false;

  public:
    SessionUDP() {}

    ~SessionUDP() {}

    template <class Packet>
    bool send(Packet& packet, const Addr& addr)
    {
      return send(packet.make_buf(*mpool_), addr);
    }

    bool send(Buf& buf, const Addr& addr)
    {
      sockaddr_in saddr = addr.sockaddr();

      int32_t ret = sendto(
        sock_,
        reinterpret_cast<const char*>(buf.buf_),
        buf.len_,
        0,
        reinterpret_cast<struct sockaddr*>(&saddr),
        sizeof(saddr)
      );

      if (ret == SOCKET_ERROR) {
        int32_t err = WSAGetLastError();
        if (err == ERROR_SUCCESS) {
          return true;
        }
        error("send", NET_FFL, err);
        return false;
      }
      return true;
    }

    IMemPool& mpool()
    {
      return *mpool_;
    }

  protected:
    virtual uint32_t recv_buf_remaining_size_to_exchange() override
    {
      return 600;
    }

    // recv 버퍼 크기
    virtual uint32_t receive_buffer_size() override
    {
      return (1024 * 8); // 8k
    }


  private:
    bool init(
      NetErrorReceiver* err_rcv, IOCP* iocp, IMemPool* mpool,
      const Addr& bind_addr, const Addr& connect_addr, void* user_data
    )
    {
      set_user_data(user_data);

      rov_.set(this);

      iocp_ = iocp;
      mpool_ = mpool;
      set_err_rcv(err_rcv);

      is_receiving_ = false;
      force_close_ = false;

      bind_addr_ = bind_addr;
      connect_addr_ = connect_addr;

      if (!sock_.open()) {
        error("session listen", NET_FFL);
        return false;
      }

      if (!sock_.disable_udp_connreset()) {
        error("session disable_udp_connreset", NET_FFL);
        return false;
      }

      if (bind_addr_) {
        if (!sock_.bind(bind_addr_)) {
          error("session bind", NET_FFL);
          return false;
        }
      }

      if (connect_addr_) {
        sockaddr_in addr = connect_addr_.sockaddr();

        int32_t ret = connect(
          sock_,
          reinterpret_cast<sockaddr*>(&addr),
          sizeof(addr)
        );
        if (ret == SOCKET_ERROR) {
          error("session connect", NET_FFL);
          return false;
        }
      }

      if (!iocp_->regist(sock_)) {
        error("session regist iocp", NET_FFL);
        return false;
      }

      rbuf_.init(
        mpool_,
        receive_buffer_size(),
        recv_buf_remaining_size_to_exchange()
      );

      return recv();
    }

    void force_close()
    {
      force_close_ = true;
      if (!*iocp_) {
        on_close();
        return;
      }

      sock_.close();
    }

    virtual void on_event(uint32_t err, OV* ov, uint32_t bytes)
    {
      switch (ov->type_) {
      case OVType::Recv:
        _on_recv(err, bytes);
        break;
      default:
        error("unknown event", NET_FFL);
        break;
      }
    }

    void _on_recv(uint32_t err, uint32_t bytes)
    {
      is_receiving_ = false;
      if (force_close_) {
        on_close();
        return;
      }

      if (err != ERROR_SUCCESS) {
        error("recv err", NET_FFL, err);
      }

      Addr client_addr = client_addr_;

      rbuf_.move_tail(bytes);

      // UDP 라서 패킷이 쪼개지거나 뭉쳐서 오는 경우가 없음
      // all or nothing or 중복 수신 가능
      uint8_t* rbuf = rbuf_.head();
      uint32_t rlen = rbuf_.data_len();

      Buf buf = rbuf_.copy_buf();
      buf.buf_ = rbuf;
      buf.len_ = rlen;

      on_recv(buf, client_addr);

      rbuf_.move_head(rlen);

      rbuf_.check_arrange();

      is_receiving_ = false;
      if (force_close_) {
        return;
      }

      recv();
    }

    void on_close()
    {
      delete this;
    }

    bool recv()
    {
      if (force_close_) {
        return true;
      }

      is_receiving_ = true;

      WSABUF wb;
      wb.buf = reinterpret_cast<CHAR*>(rbuf_.tail());
      wb.len = rbuf_.remain_len();

      DWORD recv_bytes = 0;
      DWORD flag = 0;

      rov_.reset();
      int32_t ret = WSARecvFrom(
        sock_,
        &wb,
        1,
        &recv_bytes,
        &flag,
        reinterpret_cast<sockaddr*>(&client_addr_),
        &client_addr_size_,
        &rov_,
        0
      );

      if (ret == SOCKET_ERROR) {
        int32_t err = WSAGetLastError();
        if (err != ERROR_IO_PENDING) {
          is_receiving_ = false;
          error("recv error", NET_FFL, err);
          return false;
        }
      }
      return true;
    }

    void error(const char* msg, const char* file, const char* func, int32_t line, int32_t err = WSAGetLastError())
    {
      if (bind_addr_ && connect_addr_) {
        net_error(msg, bind_addr_, connect_addr_, err, file, func, line);
      }
      else if (bind_addr_) {
        net_error(msg, bind_addr_, err, file, func, line);
      }
      else if (connect_addr_) {
        net_error(msg, connect_addr_, err, file, func, line);
      }
      else {
        net_error(msg, err, file, func, line);
      }
    }

  };

}
