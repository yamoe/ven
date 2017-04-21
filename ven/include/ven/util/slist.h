#pragma once

namespace ven {

  template <class T>
  class SList
  {
  private:
    ui32_t cnt_ = 0;
    T* start_ = nullptr;
    T* end_ = nullptr;

  public:
    SList() {}

    ~SList() {}

    SList(SList& s)
    {
      cnt_ = s.cnt_;
      start_ = s.start_;
      end_ = s.end_;
      init(s);
    }

    SList& operator=(const SList& s) = delete;

    operator bool()
    {
      return (cnt_ > 0);
    }

    T* pop()
    {
      if (cnt_ == 0) return nullptr;

      T* n = start_;
      if (cnt_ == 1) {
        start_ = end_ = nullptr;
      }
      else if (cnt_ == 2) {
        start_ = n->next_;
        end_ = start_;
      }
      else if (cnt_ > 2) {
        start_ = n->next_;
      }
      n->next_ = nullptr;
      cnt_--;
      return n;
    }

    void push(T* n)
    {
      if (cnt_ == 0) {
        start_ = n;
        end_ = n;
        n->next_ = nullptr;
      }
      else {
        n->next_ = start_;
        start_ = n;
      }
      cnt_++;
    }

    void pop(ui32_t cnt, SList& s)
    {
      if (cnt == 0) return;
      if (cnt_ == 0) return;

      ui32_t c = 1;
      T* e = start_;
      for (; c < cnt; ++c) {
        if (!e->next_) break;
        e = e->next_;
      }

      s.start_ = start_;
      s.end_ = e;
      s.cnt_ = c;

      start_ = e->next_;
      e->next_ = nullptr;
      if (!start_) end_ = nullptr;
      cnt_ -= c;
    }

    void push(SList& s)
    {
      if (s.cnt_ == 0) return;

      if (cnt_ == 0) {
        start_ = s.start_;
        end_ = s.end_;
      }
      else {
        s.end_->next_ = start_;
        start_ = s.start_;
      }
      cnt_ += s.cnt_;
      init(s);
    }

    ui32_t cnt()
    {
      return cnt_;
    }

    bool empty()
    {
      return (cnt_ == 0);
    }

  private:
    void init(SList& s)
    {
      s.cnt_ = 0;
      s.start_ = nullptr;
      s.end_ = nullptr;
    }

  };

}
