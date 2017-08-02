#pragma once

namespace ven {

  class Mem
    : public Next<Mem>
  {
  public:
    uint8_t* addr_ = nullptr;  // 할당받은 메모리
    uint32_t unit_ = 0; // 할당받은 메모리 크기

    IMemPool* mpool_ = nullptr;
    std::atomic<uint32_t> ref_{ 0 }; //uint32_t ref_ = 0;

  public:
    Mem() = default;

    void add_ref()
    {
      ref_++; //InterlockedIncrement(&ref_);
    }

    void rel_ref()
    {
      if (--ref_ == 0) {
        mpool_->ret(this);
      }
      /*
      if (InterlockedDecrement(&ref_) == 0) {
        mpool_->ret(this);
      }
      */
    }

  };

}