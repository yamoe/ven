#pragma once

namespace ven {

  template <class SessionT>
  class SessionPool
    : public ISessionPool
    , public NetErrorHandler
  {
  private:
    NetData nd_;
    void* user_data_ = nullptr;

    ui32_t pre_cnt_ = 0;  // 설정: 미리 생성
    ui32_t inc_cnt_ = 0;  // 설정: 증가 개수
    ui32_t max_cnt_ = 0;  // 설정: 최대 증가
    ui32_t total_cnt_ = 0; // 현재 생성된 개수

    SLock lock_;
    SList<SessionTCP> free_;
    std::unordered_set<SessionTCP*> use_;

  public:

    SessionPool() {}

    ~SessionPool() {}

    void init(
      NetData& nd,
      ui32_t pre_cnt,
      ui32_t inc_cnt,
      ui32_t max_cnt,
      void* user_data)
    {
      nd_ = nd;
      pre_cnt_ = pre_cnt;
      inc_cnt_ = inc_cnt;
      max_cnt_ = max_cnt;
      user_data_ = user_data;
      create(pre_cnt_);
    }

    void uninit()
    {
      VEN_BRACE_LOCKER(lock_) {
        std::vector<SessionTCP*> v(use_.begin(), use_.end());
        for (auto s : v) {
          s->disconnect();
        }
      }

      while (true) {
        Sleep(10);

        VEN_LOCKER(lock_);
        if (total_cnt_ == free_.cnt()) {
          break;
        }
      }

      VEN_LOCKER(lock_);
      while (SessionTCP* s = free_.pop()) {
        delete s;
      }
    }

    ui32_t total_cnt()
    {
      VEN_LOCKER(lock_);
      return total_cnt_;
    }

    ui32_t free_cnt()
    {
      VEN_LOCKER(lock_);
      return free_.cnt();
    }

    virtual SessionTCP* pop() override
    {
      VEN_LOCKER(lock_);

      SessionTCP* s = _pop();
      if (s) {
        use_.insert(s);
      }
      return s;
    }

    virtual void push(SessionTCP* s) override
    {
      VEN_LOCKER(lock_);

      use_.erase(s);

      //if (free_.cnt() >= pre_cnt_) {
      //  total_cnt_--;
      //  delete s;
      //  return;
      //}

      free_.push(s);
    }

  private:
    SessionTCP* _pop()
    {
      SessionTCP* s = free_.pop();
      if (s) return s;

      ui32_t cnt = create_cnt();
      if (cnt == 0) {
        return nullptr;
      }
      create(cnt);

      return free_.pop();
    }

    ui32_t create_cnt()
    {
      if (total_cnt_ >= max_cnt_) {
        return 0;
      }

      ui32_t diff = max_cnt_ - total_cnt_;
      if (diff >= inc_cnt_) {
        return inc_cnt_;
      }
      return diff;
    }

    void create(ui32_t cnt)
    {
      if (cnt == 0) return;

      total_cnt_ += cnt;
      for (ui32_t i = 0; i < cnt; ++i) {
        SessionTCP* s = static_cast<SessionTCP*>(new SessionT);
        s->set_init(err_rcv(), nd_, user_data_);
        free_.push(s);
      }
    }

  };

}
