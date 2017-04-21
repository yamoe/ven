#pragma once

namespace ven {

  class SessionTCP
    : public ISessionTCP
    , public Actor
    , public Next<SessionTCP>
    , public NetErrorHandler
    , public UserData
  {
  private:
    friend class Acceptor;
    friend class Net;
    template <class SessionT> friend class SessionPool;

    SocketTCP sock_;
    ROV rov_ = ROV(this);
    COV cov_ = COV(this);
    DOV dov_ = DOV(this);
    RBuf rbuf_;
    SBuf sbuf_;

    Addr remote_addr_;
    NetData nd_;

    SLock s_lock_; //send
    SLock cd_lock_; //conn, disconn lock

    std::atomic<bool> is_connected_ = false;
    std::atomic<bool> is_receiving_ = false;

    bool is_connecting_ = false;
    bool is_disconnecting_ = false;
    bool is_force_disconnecting_ = false;
    bool is_release_ = false;

  public:
    SessionTCP() {}

    virtual ~SessionTCP() {}

    bool connect()
    {
      VEN_LOCKER(cd_lock_);

      if (is_server_session()) return false;
      if (is_connected_) return true;
      if (is_connecting_) return true;
      is_connecting_ = true;

      cov_.reset();
      bool ret = sock_.connectex(&cov_, remote_addr_);
      if (!ret) {
        int_t err = WSAGetLastError();
        if (err != ERROR_IO_PENDING) {
          return false;
        }
      }
      return true;
    }

    // release true 인 경우 연결을 끊고 delete 수행
    void disconnect(bool is_release = false)
    {
      VEN_LOCKER(cd_lock_);
      is_release_ = is_release;

      if (!is_connected_) {
        return;
      }

      if (is_force_disconnecting_) {
        return;
      }
      is_force_disconnecting_ = true;

      // TIME_WAIT가 안남도록 DisconnectEx 대신 소켓 종료
      if (!sock_.set_linger()) {
        error("session(client) linger", NET_FFL);
      }
      sock_.close();

      // recv, send 가 걸려있지 않은 경우를 위해 호출
      _disconnect();
    }

    template <class Packet>
    bool send(Packet& packet)
    {
      return send(packet.make_buf(*nd_.mpool_));
    }

    bool send(Buf& buf)
    {
      if (!is_connected_) {
        return false;
      }

      SOV* sov = nullptr;

      VEN_BRACE_LOCKER(s_lock_) {
        if (!sbuf_.push(buf)) return false;
        sov = sbuf_.pop();
      }

      if (!sov) {
        return true;
      }

      return send(sov);
    }


    MemPool& mpool()
    {
      return *nd_.mpool_;
    }

  protected:
    // 수신 패킷 사이즈
    virtual bool get_packet_size(byte_t* buf, const ui32_t len, ui32_t& packet_size) override
    {
      if (len < header_size()) return false;
      packet_size = *(reinterpret_cast<ui32_t*>(buf));
      return true;
    }

    // 이상한 패킷 사이즈인 경우 연결 종료 수행
    virtual bool is_valid_packet_size(ui32_t size) override
    {
      return (size != 0) && (size <= (1024 * 128));
    }


    // 패킷의 헤더 사이즈(bytes)
    virtual ui32_t header_size() override
    {
      return 4;
    }

    // 수신 버퍼 크기
    virtual ui32_t receive_buffer_size() override
    {
      return (1024 * 8);
    }

    // 연결
    virtual void on_conn() override {}

    // 연결 종료
    virtual void on_disc(ui32_t err) override {}

  private:
    void set_init(NetErrorReceiver* err_rcv, NetData& nd, void* user_data) {
      set_err_rcv(err_rcv);
      nd_ = nd;
      set_user_data(user_data);
    }

    bool is_server_session()
    {
      return nd_.acpt_ != nullptr;
    }

    bool init_client_session(const Addr& addr, void* user_data)
    {
      remote_addr_ = addr;
      set_user_data(user_data);

      init_client_socket();
      return true;
    }

    void init_client_socket()
    {
      if (!sock_.bind()) {
        error("session bind", NET_FFL);
      }
      regist_iocp();
    }

    virtual void on_event(err_t err, OV* ov, ui32_t bytes) override
    {
      switch (ov->type_) {
      case OVType::Conn: _on_conn(err); break;
      case OVType::Disc: _on_disc(); break;
      case OVType::Send: _on_send(err, ov, bytes); break;
      case OVType::Recv: _on_recv(err, bytes); break;
      default:
        error("unknown event", NET_FFL);
        break;
      }
    }

    void _on_accept(Socket& listen_sock, const Addr& remote_addr)
    {
      remote_addr_ = remote_addr;

      if (!sock_.update_accept_context(listen_sock)) {
        error("session update_accept_context", NET_FFL);
        disconnect();
        return;
      }

      regist_iocp();
      _on_conn_init();
    }

    void _on_conn_init()
    {
      is_connected_ = true;
      is_receiving_ = false;
      is_connecting_ = false;
      is_force_disconnecting_ = false;
      is_disconnecting_ = false;

      sbuf_.init(nd_.sov_pool_, this);
      rbuf_.init(
        nd_.mpool_,
        receive_buffer_size(),
        header_size()
      );

      on_conn();
      recv();
    }

    void regist_iocp()
    {
      if (sock_.registered_iocp()) {
        return;
      }

      if (nd_.iocp_->regist(sock_)) {
        sock_.set_registered_iocp(true);
      }
      else {
        error("session regist iocp", NET_FFL);
      }
    }

    void _on_disc()
    {
      VEN_LOCKER(cd_lock_);

      sbuf_.uninit();
      rbuf_.uninit();

      on_disc(dov_.err_);

      // 강제 종료한 경우 socket 재생성
      if (is_force_disconnecting_) {
        sock_.open();

        if (!is_server_session()) {
          init_client_socket();
        }
      }

      if (is_release_) {
        delete this;
      } else {
        if (is_server_session()) {
          nd_.acpt_->disconnected(
            reinterpret_cast<SessionTCP*>(this)
          );
        }
      }
      
    }

    void _on_recv(err_t err, ui32_t bytes)
    {
      if (err != ERROR_SUCCESS || bytes == 0) {
        is_receiving_ = false;
        _disconnect(err);
        return;
      }

      rbuf_.move_tail(bytes);

      while (true) {
        byte_t* rbuf = rbuf_.head();
        ui32_t rlen = rbuf_.data_len();
        ui32_t psize = 0;

        if (get_packet_size(rbuf, rlen, psize)) {
          if (!is_valid_packet_size(psize)) {
            is_receiving_ = false;
            _disconnect(16000);
            return;
          }

          if (psize <= rlen) {
            Buf buf = rbuf_.copy_buf();
            buf.buf_ = rbuf;
            buf.len_ = psize;

            on_recv(buf);

            rbuf_.move_head(psize);
            continue;

          }
          else {
            // 더 받아야 하는 경우
            rbuf_.check_size(psize);
          }
        }

        rbuf_.check_arrange();


        is_receiving_ = false;
        if (!is_connected_) {
          _disconnect();
          return;
        }

        recv();
        return;
      }
    }

    void _on_send(err_t err, OV* ov, ui32_t bytes)
    {
      SOV* sov = static_cast<SOV*>(ov);
      sov->slist_.clear();

      bool has_to_send = false;
      VEN_BRACE_LOCKER(s_lock_) {
        sbuf_.push(sov);
        has_to_send = sbuf_.has_to_send();
      }

      if (err != ERROR_SUCCESS || !is_connected_) {
        _disconnect(err);
        return;
      }

      if (has_to_send) {
        send(Buf());
      }
    }

    void recv()
    {
      if (!is_connected_) {
        return;
      }
      is_receiving_ = true;

      WSABUF wb;
      wb.buf = reinterpret_cast<CHAR*>(rbuf_.tail());
      wb.len = rbuf_.remain_len();

      DWORD recv_bytes = 0;
      DWORD flag = 0;

      rov_.reset();

      int ret = WSARecv(
        sock_,
        &wb,
        1,
        &recv_bytes,
        &flag,
        &rov_,
        0
      );

      if (ret == SOCKET_ERROR) {
        int_t err = WSAGetLastError();
        if (err != ERROR_IO_PENDING) {
          is_receiving_ = false;
          _disconnect(err);
          return;
        }
      }
    }

    bool send(SOV* sov)
    {
      DWORD sbytes = 0;
      sov->reset();
      int_t ret = WSASend(
        sock_,
        sov->slist_.wsabuf(),
        sov->slist_.size(),
        &sbytes,
        0,
        sov,
        0
      );

      if (ret == SOCKET_ERROR) {
        int_t err = WSAGetLastError();
        if (err != ERROR_IO_PENDING) {
          _on_send(err, sov, 0);
          return false;
        }
      }
      return true;
    }

    void _disconnect(ui32_t err = 0)
    {
      VEN_LOCKER(cd_lock_);

      if (is_connected_) {
        dov_.reset(err);
      }
      is_connected_ = false;

      bool is_sending = false;
      VEN_BRACE_LOCKER(s_lock_) {
        is_sending = sbuf_.is_sending();
      }

      if (is_sending || is_receiving_) {
        return;
      }

      if (is_disconnecting_) {
        return;
      }
      is_disconnecting_ = true;

      if (is_force_disconnecting_) {
        _on_disc();
        return;
      }

      if (!sock_.set_linger()) {
        error("session linger", NET_FFL);
      }

      if (!sock_.disconnectex(&dov_)) {
        int_t err = WSAGetLastError();
        if (err != ERROR_IO_PENDING) {
          error("session disconnectex", NET_FFL);
          return;
        }
      }

    }

    void _on_conn(err_t err)
    {
      VEN_LOCKER(cd_lock_);

      is_connecting_ = false;

      if (err != ERROR_SUCCESS) {
        on_disc(err);
        return;
      }

      if (!sock_.update_connect_context()) {
        error("session update connect context", NET_FFL);
      }
      _on_conn_init();
    }

    void error(cchar_t* msg, cchar_t* file, cchar_t* func, int_t line)
    {
      net_error(msg, remote_addr_, WSAGetLastError(), file, func, line);
    }

  };


}
