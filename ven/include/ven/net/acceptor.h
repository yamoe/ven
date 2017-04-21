#pragma once

namespace ven {

  class Acceptor
    : public Actor
    , public IAcceptor
    , public NetErrorHandler
  {
  private:
    NetData nd_;
    ISessionPool* spool_ = nullptr;

    SocketListen sock_;
    Addr listen_addr_;

    SListLocker<AOV> apool_;
    std::atomic<ui32_t> accepting_cnt_ = 0;
    std::atomic<ui32_t> connected_cnt_ = 0;
    std::atomic<bool> is_release_ = false;

  public:
    Acceptor() {}

    virtual ~Acceptor() {}

    void init(
      NetData& nd,
      ISessionPool* spool,
      ui32_t accept_cnt
    ) {
      nd_ = nd;
      spool_ = spool;
      for (ui32_t i = 0; i < accept_cnt; ++i) {
        AOV* aov = new AOV;
        aov->set(this);
        apool_.push(aov);
      }
    }

    void uninit()
    {
      is_release_ = true;
      sock_.close();
      while (accepting_cnt_ != 0) {
        Sleep(10);
      }

      while (AOV* aov = apool_.pop()) {
        delete aov;
      }
    }

    ui32_t accepting_cnt()
    {
      return accepting_cnt_;
    }

    ui32_t connected_cnt()
    {
      return connected_cnt_;
    }

    bool listen(const Addr& addr)
    {
      listen_addr_ = addr;

      if (!sock_.set_reuse(true)) {
        error("listen reuse socket", NET_FFL);
        return false;
      }

      if (!sock_.set_conditional_accept(true)) {
        error("listen conditional accept", NET_FFL);
        return false;
      }

      if (!sock_.bind(listen_addr_)) {
        error("listen bind", NET_FFL);
        return false;
      }

      if (!sock_.listen()) {
        error("listen", NET_FFL);
        return false;
      }

      if (!nd_.iocp_->regist(sock_)) {
        error("listen regist iocp", NET_FFL);
        return false;
      }

      accept_all();
      return true;
    }

    virtual void disconnected(SessionTCP* s) override
    {
      connected_cnt_--;
      spool_->push(s);
      accept_all();
    }

    void accept_all()
    {
      if (is_release_) return;

      while (!apool_.empty()) {
        if (!accept()) return;
      }
    }

    bool accept()
    {
      if (is_release_) return false;

      AOV* aov = apool_.pop();
      if (!aov) return false;

      SessionTCP* s = spool_->pop();
      if (!s) {
        apool_.push(aov);
        return false;
      }

      aov->reset(s);
      if (!sock_.acceptex(aov, s->sock_, aov->buf_)) {
        int_t err = WSAGetLastError();
        if (err != ERROR_IO_PENDING) {
          return false;
        }
      }

      accepting_cnt_++;
      return true;
    }

    virtual void on_event(err_t err, OV* ov, ui32_t bytes) override
    {
      AOV* aov = static_cast<AOV*>(ov);

      if (err != ERROR_SUCCESS) {
        spool_->push(aov->sess_);

        if (!is_release_) {
          error("listen on accept", NET_FFL);
        }
      } else {
        connected_cnt_++;

        Addr remote_addr;
        sock_.getacceptexsockaddrs(aov->buf_, remote_addr);

        aov->sess_->_on_accept(sock_, remote_addr);
      }

      accepting_cnt_--;

      apool_.push(aov);
      accept();
    }
    private:
      void error(cchar_t* msg, cchar_t* file, cchar_t* func, int_t line, int_t err = WSAGetLastError())
      {
        net_error(msg, listen_addr_, err, file, func, line);
      }
  };

}
