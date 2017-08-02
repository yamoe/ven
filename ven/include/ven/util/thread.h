#pragma once

namespace ven {

  class Thread : public NonCopyable {
  private:
    uint64_t thread_id_ = 0;
#if defined(WINDOWS)
    HANDLE th_ = nullptr;
#else
    std::thread th_;
#endif


  public:
    Thread() {}

    virtual ~Thread() {}

    void start()
    {
#if defined(WINDOWS)
      if (th_) return;
      uint32_t tid = 0;
      uintptr_t handle = _beginthreadex(
        0,
        0,
        (unsigned(__stdcall*)(void*))run_thread,
        this,
        0,
        &tid
      );
      thread_id_ = static_cast<uint64_t>(tid);
      th_ = reinterpret_cast<HANDLE>(handle);
#else
      th_ = std::thread(&Thread::run, this);
      thread_id_ = ven::tid(th_);
#endif
    }

    void join()
    {
#if defined(WINDOWS)
      if (!th_) return;
      WaitForSingleObject(th_, INFINITE);
#else
      th_.join();
#endif
    }

    uint64_t thread_id()
    {
      return thread_id_;
    }

  protected:
    virtual void run() = 0;

#if defined(WINDOWS)
  private:
    static uintptr_t __stdcall run_thread(void* p)
    {
      Thread* th = static_cast<Thread*>(p);
      th->run();
      th->close();
      return 0;
    }

    void close()
    {
      if (th_) {
        CloseHandle(th_);
        th_ = nullptr;
      }
    }
#endif

  };

}

