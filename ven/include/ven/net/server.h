#pragma once

namespace ven {

  template <class SessionT>
  class Server
    : public IServer
  {
  private:
    IOCP& iocp_;
    MemPool& mpool_;
    SOVPool& sov_pool_;

    SessionPool<SessionT> spool_;
    Acceptor acpt_;

    Addr addr_;

    NetData nd_;

  public:
    Server(
      NetErrorReceiver* err_rcv,
      IOCP& iocp,
      MemPool& mpool,
      SOVPool& sov_pool,
      ServerConf& conf
    )
      : iocp_(iocp)
      , mpool_(mpool)
      , sov_pool_(sov_pool)
    {
      addr_ = conf.addr_;

      nd_.iocp_ = &iocp; 
      nd_.mpool_ = &mpool_;
      nd_.sov_pool_ = &sov_pool;
      nd_.acpt_ = &acpt_;

      spool_.set_err_rcv(err_rcv);
      spool_.init(
        nd_,
        conf.session_pre_cnt_,
        conf.session_inc_cnt_,
        conf.session_max_cnt_,
        conf.session_user_data_
      );

      acpt_.set_err_rcv(err_rcv);
      acpt_.init(
        nd_,
        &spool_,
        conf.accept_cnt_
      );
    }

    virtual ~Server() {}

    virtual bool run() override
    {
      return acpt_.listen(addr_);
    }

    virtual void stop() override
    {
      acpt_.uninit();
      spool_.uninit();
    }

    virtual void state(SvrState& s) override
    {
      s.total_ = spool_.total_cnt();
      s.free_ = spool_.free_cnt();
      s.conn_ = acpt_.connected_cnt();
      s.accepting_ = acpt_.accepting_cnt();
    }

  };

}
