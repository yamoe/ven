#pragma once

namespace ven {

  class SBuf {
  private:
    Actor* actor_ = nullptr;
    SOVPool* pool_ = nullptr;
    SOV* sov_ = nullptr;

    ui32_t pop_cnt_ = 0;

  public:
    SBuf() {}

    ~SBuf()
    {
      uninit();
    }

    void init(SOVPool* pool, Actor* actor)
    {
      actor_ = actor;
      pool_ = pool;
      sov_ = alloc();
      pop_cnt_ = 0;
    }

    void uninit()
    {
      if (sov_) {
        sov_->slist_.clear();
        pool_->push(sov_);
        sov_ = nullptr;
      }
      actor_ = nullptr;
      pool_ = nullptr;
      pop_cnt_ = 0;
    }

    bool push(Buf& buf)
    {
      if (!buf) return true;

      sov_->slist_.push_back(buf);
      return true;
    }

    SOV* pop()
    {
      if (sov_->slist_.empty()) {
        return nullptr;
      }

      if (!is_sending() || sov_->slist_.is_full()) {
        pop_cnt_++;
        return get();
      }

      return nullptr;
    }

    bool is_sending()
    {
      return (pop_cnt_ > 0);
    }

    bool has_to_send()
    {
      return (!is_sending() && !sov_->slist_.empty());
    }

    void push(SOV* sov)
    {
      pop_cnt_--;
      pool_->push(sov);
    }

  private:
    SOV* get()
    {
      SOV* sov = alloc();
      std::swap(sov_, sov);
      return sov;
    }

    SOV* alloc()
    {
      SOV* sov = pool_->pop();
      sov->set(actor_);
      return sov;
    }

  };

}
