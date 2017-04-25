#pragma once

namespace ven {

  template <class T>
  class PoolPtr {
  public:
    typedef std::deque<T*> List;

  private:
    SLock lock_;
    List list_;

  public:
    PoolPtr() {}

    ~PoolPtr()
    {
      VEN_LOCKER(lock_);
      for (auto& v : list_) {
        delete v;
      }
    }

    T* pop()
    {
      VEN_LOCKER(lock_);
      if (list_.empty()) {
        return nullptr;
      }

      T* t = list_.front();
      list_.pop_front();
      return t;
    }

    void push(T* t)
    {
      VEN_LOCKER(lock_);
      list_.push_back(t);
    }

    bool empty()
    {
      VEN_LOCKER(lock_);
      return list_.empty();
    }

  };

}