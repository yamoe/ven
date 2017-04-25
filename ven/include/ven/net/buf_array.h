#pragma once

namespace ven {

  class BufArray {
  private:
    ui32_t capacity_ = 0;
    ui32_t next_idx_ = 0;

    byte_t* m_ = nullptr;
    WSABUF* wbs_ = nullptr;
    Mem** mems_ = nullptr;

  public:
    BufArray() {}
    BufArray(const BufArray&) = delete;
    void operator=(const BufArray& c) = delete;

    ~BufArray()
    {
      uninit();
    }

    void reserve(ui32_t size)
    {
      if (m_) return;

      next_idx_ = 0;
      capacity_ = size;

      ui32_t malloc_size = capacity_ * (sizeof(WSABUF) + sizeof(Mem*));
      m_ = static_cast<byte_t*>(
        malloc(malloc_size)
      );

      wbs_ = reinterpret_cast<WSABUF*>(m_);
      mems_ = reinterpret_cast<Mem**>(m_ + (capacity_ * sizeof(WSABUF)));
    }

    void push_back(Buf& buf)
    {
      if (is_full()) {
        throw std::out_of_range("array is full");
      }

      WSABUF& wb = wbs_[next_idx_];
      wb.buf = reinterpret_cast<CHAR*>(buf.buf_);
      wb.len = buf.len_;

      Mem*& mem = mems_[next_idx_];
      mem = buf.dettach();;
      next_idx_++;
    }

    void uninit()
    {
      clear();
      free(m_);
      m_ = nullptr;
      wbs_ = nullptr;
      mems_ = nullptr;
    }

    void swap(BufArray& cls)
    {
      std::swap(capacity_, cls.capacity_);
      std::swap(next_idx_, cls.next_idx_);
      std::swap(wbs_, cls.wbs_);
      std::swap(mems_, cls.mems_);
    }

    bool is_full()
    {
      return (capacity_ == next_idx_);
    }

    bool empty()
    {
      return (next_idx_ == 0);
    }

    ui32_t size()
    {
      return next_idx_;
    }

    void clear()
    {
      for (ui32_t i = 0; i < next_idx_; ++i) {
        mems_[i]->rel_ref();
      }
      next_idx_ = 0;
    }

    WSABUF* wsabuf()
    {
      return wbs_;
    }
  };

}
