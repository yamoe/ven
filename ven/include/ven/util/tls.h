#pragma once

namespace ven {

  template <class T = void*>
  class Tls {
  private:
    DWORD idx_ = TLS_OUT_OF_INDEXES;

  public:
    Tls()
    {
      init();
    }

    ~Tls()
    {
      uninit();
    }

    void set(T* val)
    {
      void* v = static_cast<void*>(val);
      TlsSetValue(idx_, v);
    }

    T* get()
    {
      void* v = TlsGetValue(idx_);
      return static_cast<T*>(v);
    }

  private:
    bool init()
    {
      if (idx_ != TLS_OUT_OF_INDEXES) {
        return true;
      }

      idx_ = TlsAlloc();
      return (idx_ != TLS_OUT_OF_INDEXES);
    }

    void uninit()
    {
      if (idx_) {
        TlsFree(idx_);
        idx_ = TLS_OUT_OF_INDEXES;
      }
    }

  };

}

