#pragma once

namespace ven {

  /*
  lock free LIFO stack
  구현이 정확하지 않음
  Thread 경합이 많은 경우 CPU 사용량이 훨씬 많음
  */
  template <class T>
  class LFStack
  {
  private:
    struct Data
    {
      T* ptr_ = nullptr;
      size_t pop_cnt_ = 0;
    };

    Data* head_ = nullptr;
    std::atomic<ui32_t> cnt_ = 0;

  public:
    LFStack()
    {
      head_ = (Data*)_aligned_malloc(sizeof(Data), 16);
      head_->ptr_ = nullptr;
      head_->pop_cnt_ = 0;
    }

    ~LFStack()
    {
      if (head_) {
        _aligned_free(head_);
        head_ = nullptr;
      }
    }

    LFStack(LFStack& s) = delete;

    LFStack& operator=(const LFStack& s) = delete;

    void push(T* n)
    {
      while (true) {
         T* ptr = head_->ptr_;
         n->next_ = ptr;
        if (InterlockedCompareExchangePointer(
          (PVOID*)&head_->ptr_,
          (PVOID)n,
          (PVOID)ptr
        ) == (PVOID)ptr) {
          cnt_++;
          break;
        }
      }
    }

    T* pop()
    {
      while (true) {
        T* ptr = head_->ptr_;
        size_t pop_cnt = head_->pop_cnt_;
        if (!ptr) {
          return nullptr;
        }

        T* next = ptr->next_;
        if (CAS2(head_, ptr, pop_cnt, next, pop_cnt + 1)) {
          cnt_--;
          ptr->next_ = nullptr;
          return ptr;
        }
      }
    }

    ui32_t cnt()
    {
      return cnt_;
    }

  private:
#ifdef _WIN64
    bool CAS2(Data* data, T* old1, size_t old2, T* new1, size_t new2)
    {
      Data old;
      old.ptr_ = old1;
      old.pop_cnt_ = old2;
      return (InterlockedCompareExchange128(
        reinterpret_cast<LONG64 volatile *>(data),
        static_cast<LONG64>(new2),
        reinterpret_cast<LONG64>(new1),
        reinterpret_cast<LONG64*>(&old)
      ) == 1);
    }
#else
    bool CAS2(Data* data, T* old1, size_t old2, T* new1, size_t new2)
    {
      LONGLONG comparand = reinterpret_cast<long>(old1) | (static_cast<LONGLONG>(old2) << 32);
      LONGLONG exchange = reinterpret_cast<long>(new1) | (static_cast<LONGLONG>(new2) << 32);

      return (InterlockedCompareExchange64(
        reinterpret_cast<LONGLONG volatile *>(data),
        exchange,
        comparand
      ) == comparand);
    }
#endif

  };

}
