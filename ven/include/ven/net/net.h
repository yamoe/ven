#pragma once

namespace ven {

  class Net
    : public NetErrorHandler
  {
  private:
    NetConf nconf_;

    SLock lock_;
    AddrUnorderedMap<IServer*> tcp_svrs_;
    std::unordered_set<SessionTCP*> tcp_clis_;

    IOCP iocp_;
    std::vector<IOThread*> ths_;
    SOVPool sov_pool_;
    Auto<IMemPool> mpool_;
    Auto<NetErrorReceiver> err_rcv_;

  public:

    Net(NetConf nconf = NetConf())
      : nconf_(nconf)
    {
      init_err_rcv();
      init_winsock();
      init_iocp();
      init_memory();
      init_sov_pool();
      init_thread();
    }

    ~Net()
    {
      uninit();
    }

    template <class SessionT>
    bool run_tcp_server(ServerConf conf = ServerConf()) 
    {
      check_tcp_class<SessionT>();

      VEN_LOCKER(lock_);

      if (tcp_svrs_.find(conf.addr_) != tcp_svrs_.end()) {
        return false;
      }

      IServer* svr = reinterpret_cast<IServer*>(
        new Server<SessionT>(
          err_rcv(),
          iocp_,
          *mpool_,
          sov_pool_,
          conf
        )
      );

      if (!svr->run()) {
        delete svr;
        return false;
      }

      tcp_svrs_.insert(std::make_pair(conf.addr_, svr));

      return true;
    }

    bool stop_tcp_server(const Addr& addr)
    {
      VEN_LOCKER(lock_);

      auto it = tcp_svrs_.find(addr);
      if (it == tcp_svrs_.end()) {
        return false;
      }

      IServer* svr = it->second;
      tcp_svrs_.erase(it);

      svr->stop();
      delete svr;
      
      return true;
    }

    template <class SessionT>
    SessionT* run_tcp_client(const Addr& addr, void* user_data = nullptr)
    {
      check_tcp_class<SessionT>();

      VEN_LOCKER(lock_);
      SessionTCP* s = static_cast<SessionTCP*>(new SessionT);

      NetData nd;
      nd.iocp_ = &iocp_;
      nd.mpool_ = mpool_;
      nd.sov_pool_ = &sov_pool_;

      s->set_init(err_rcv(), nd, user_data);
      s->init_client_session(addr, user_data);

      tcp_clis_.insert(s);
      return static_cast<SessionT*>(s);
    }

    // 연결 종료 및 Session 객체 delete
    template <class SessionT>
    void stop_tcp_client(SessionT*& s)
    {
      check_tcp_class<SessionT>();

      if (!s) return;

      tcp_clis_.erase(reinterpret_cast<Session*>(s));
      s->disconnect(true);
	  s = nullptr;
    }

    template <class SessionT>
    SessionT* run_udp(
      const Addr& bind_addr,
      const Addr& connect_addr = Addr(),
      void* user_data = nullptr
    )
    {
      check_ucp_class<SessionT>();

      VEN_LOCKER(lock_);

      if (!bind_addr && !connect_addr) {
        net_error("run_udp. addr is empty", NET_FFL);
        return false;
      }

      SessionUDP* s = reinterpret_cast<SessionUDP*>(new SessionT);

      if (!s->init(
        err_rcv(), &iocp_, mpool_,
        bind_addr, connect_addr, user_data
      )) {
        delete s;
        return nullptr;
      }

      return reinterpret_cast<SessionT*>(s);
    }

    // 연결 종료 및 Session 객체 delete
    template <class SessionT>
    void stop_udp(SessionT*& s)
    {
      check_ucp_class<SessionT>();

      if (!s) return;

      iocp_.uninit();
      s->force_close();
      s = nullptr;
    }

    NetState state()
    {
      NetState s;

      s.mem_ = mpool_->state();

      s.sov_.total_ = sov_pool_.total_cnt();
      s.sov_.free_ = sov_pool_.free_cnt();

      for (auto& kv : tcp_svrs_) {
        auto& addr = kv.first;
        auto& svr = kv.second;
        svr->state(s.svr_[addr]);
      }
      return s;
    }

    IMemPool& mpool()
    {
      return *mpool_;
    }

  private:
    
    template <class SessionT>
    void check_tcp_class()
    {
      static_assert(std::is_base_of<SessionTCP, SessionT>::value, "not a class derived from SessionTCP");
    }

    template <class SessionT>
    void check_ucp_class()
    {
      static_assert(std::is_base_of<SessionUDP, SessionT>::value, "not a class derived from SessionUDP");
    }

    void init_winsock()
    {
      if (WSAStartup(MAKEWORD(2, 2), &WSADATA()) == SOCKET_ERROR) {
        net_error("WSACleanup", NET_FFL);
      }
    }

    void uninit_winsock()
    {
      if (WSACleanup() == SOCKET_ERROR) {
        net_error("WSACleanup", NET_FFL);
      }
    }

    std::string address(const std::string& ip, uint16_t port)
    {
      return make_str("%s:%d", ip.c_str(), port);
    }

    void uninit()
    {
      VEN_LOCKER(lock_);

      for (auto& kv : tcp_svrs_) {
        IServer* svr = kv.second;
        svr->stop();
        delete svr;
      }
      tcp_svrs_.clear();

      for (auto& s : tcp_clis_) {
        s->disconnect(true);
      }
      tcp_clis_.clear();

      // iocp
      iocp_.uninit();

      // thread
      for (auto& th : ths_) {
        //th->join();
		ven::sleep(10);
        delete th;
      }

      mpool_.reset();
      sov_pool_.uninit();

      uninit_winsock();
    }

    void init_err_rcv()
    {
      if (nconf_.err_rcv_) {
        err_rcv_ = nconf_.err_rcv_;
        err_rcv_.not_mine();
      }
      else {
        err_rcv_ = new NetErrorReceiver;
      }
      set_err_rcv(err_rcv_);
    }

    void init_iocp()
    {
      iocp_.init();
      iocp_.set_err_rcv(err_rcv());
    }

    void init_memory()
    {
      if (nconf_.mpool_) {
        mpool_ = nconf_.mpool_;
        mpool_.not_mine();
      }
      else {
        MemPool* mpool = new MemPool;
        mpool->init({
          { 1 * 1024,{ 100, 100 } }, // 1k 100개 생성. 부족시 100개 생성
          { 2 * 1024,{ 100, 100 } }, // 2k 100개
          { 4 * 1024,{ 100, 100 } }, // 4k 100개
          { 8 * 1024,{ 1000, 100 } }, // 8k 100개
        });
        mpool_ = mpool;
      }
    }

    void init_sov_pool()
    {
      sov_pool_.init(
        nconf_.sov_pool_init_size_,
        nconf_.sov_pool_step_size_,
        nconf_.sov_pool_arr_size_
      );
    }

    void init_thread()
    {
      uint32_t thread_cnt = nconf_.thread_cnt_;

      if (thread_cnt == 0) {
        thread_cnt = (cpu_count() * 2) + 1;
      }

      for (uint32_t i = 0; i < thread_cnt; ++i) {
        IOThread* th = new IOThread(iocp_);
        th->set_err_rcv(err_rcv());
        ths_.push_back(th);
        th->start();
      }
    }

  };

}

