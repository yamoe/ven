#pragma once

namespace ven {

  template <class T>
  class SListLocker
  {
  private:
    SLock lock_;
    SList<T> s_;

  public:
    SListLocker() {}

    SListLocker(SList<T>& list)
      : s_(list)
    {}

    SLock& lock()
    {
      return lock_;
    }

    uint32_t cnt()
    {
      VEN_LOCKER(lock_);
      return s_.cnt();
    }

    bool empty()
    {
      VEN_LOCKER(lock_);
      return s_.empty();
    }

    T* pop()
    {
      VEN_LOCKER(lock_);
      return s_.pop();
    }

    void push(T* t)
    {
      VEN_LOCKER(lock_);
      s_.push(t);
    }

    void pop(uint32_t cnt, SList<T>& list)
    {
      VEN_LOCKER(lock_);
      s_.pop(cnt, list);
    }

    void push(SList<T>& list)
    {
      VEN_LOCKER(lock_);
      s_.push(list);
    }
  };

}
