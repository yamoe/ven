#pragma once

namespace ven {
  namespace test {

    class Buf {
    public:
      byte_t* buf_ = nullptr;
      ui32_t len_ = 0;

    private:
      Mem* mem_ = nullptr;

    public:
      Buf() {}

      Buf(Mem* mem)
        : mem_(mem)
      {
        if (mem_) {
          mem_->add_ref();
          buf_ = mem_->addr_;
          len_ = mem_->unit_;
        }
      }

      Buf(const Buf& cls)
        : mem_(cls.mem_)
      {
        if (mem_) {
          mem_->add_ref();
          buf_ = cls.buf_;
          len_ = cls.len_;
        }
      }

      ~Buf()
      {
        if (mem_) mem_->rel_ref();
      }

      Buf& operator=(const Buf& cls)
      {
        if (this == &cls) {
          return *this;
        }

        if (mem_) mem_->rel_ref();
        mem_ = cls.mem_;
        buf_ = cls.buf_;
        len_ = cls.len_;
        if (mem_) mem_->add_ref();
        return *this;
      }

      void swap(Buf& b)
      {
        std::swap(buf_, b.buf_);
        std::swap(len_, b.len_);
        std::swap(mem_, b.mem_);
      }

      Mem* dettach()
      {
        Mem* mem = mem_;
        mem_ = nullptr;
        buf_ = nullptr;
        len_ = 0;
        return mem;
      }

      Mem* operator->() const
      {
        return mem_;
      }

      operator bool() const
      {
        return (mem_ != nullptr);
      }

    };

  }
}