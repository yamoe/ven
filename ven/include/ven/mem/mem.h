#pragma once

namespace ven {

  class Mem
    : public Next<Mem>
  {
  public:
    byte_t* addr_ = nullptr;  // 할당받은 메모리
    unit_t unit_ = 0; // 할당받은 메모리 크기

    IMemPool* mpool_ = nullptr;
    ui32_t ref_ = 0;

  public:
    Mem() = default;

    void add_ref()
    {
      InterlockedIncrement(&ref_);
    }

    void rel_ref()
    {
      if (InterlockedDecrement(&ref_) == 0) {
        mpool_->ret(this);
      }
    }

  };

}