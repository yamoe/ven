#pragma once

namespace ven {

  class SOVPool {
  private:
    uint32_t init_cnt_ = 100; // 초기 생성 수
    uint32_t step_cnt_ = 100; // 증감 단위
    uint32_t del_cnt_ = 100; // 초기 생성 수
    uint32_t arr_cnt_ = 5000; // SOV 당 BufArray 개수

    SLock lock_;
    SList<SOV> free_;
    uint32_t total_cnt_ = 0;

  public:
    SOVPool() {}

    ~SOVPool() { uninit(); }

    void init(uint32_t init_cnt, uint32_t step_cnt, uint32_t arr_cnt)
    {
      VEN_LOCKER(lock_);
      init_cnt_ = init_cnt;
      step_cnt_ = step_cnt;
      del_cnt_ = init_cnt * 2;
      arr_cnt_ = arr_cnt;

      create(init_cnt_);
    }

    void uninit()
    {
      VEN_LOCKER(lock_);
      while (SOV* sov = free_.pop()) {
        delete sov;
      }
    }

    uint32_t total_cnt()
    {
      return total_cnt_;
    }

    uint32_t free_cnt()
    {
      return free_.cnt();
    }

    SOV* pop()
    {
      VEN_LOCKER(lock_);

      SOV* sov = _pop();
      if (sov) return sov;
      
      create(step_cnt_);
      return _pop();
    }

    void push(SOV* sov)
    {
      VEN_LOCKER(lock_);

      //if (free_.cnt() >= del_cnt_) {
      //  total_cnt_--;
      //  delete sov;
      //  return;
      //}
      _push(sov);
    }

  private:
    SOV* _pop()
    {
      return free_.pop();
    }

    void _push(SOV* sov)
    {
      free_.push(sov);
    }

    void create(uint32_t cnt)
    {
      total_cnt_ += cnt;
      for (uint32_t i = 0; i < cnt; ++i) {
        _push(create());
      }
    }

    SOV* create()
    {
      SOV* sov = new SOV;
      sov->slist_.reserve(arr_cnt_);
      return sov;
    }

  };

}
