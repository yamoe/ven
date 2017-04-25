#pragma once

namespace ven {

  class Thread : public NonCopyable {
  private:
    HANDLE handle_ = nullptr;
    ui32_t thread_id_ = 0;

  public:
    Thread() {}

    virtual ~Thread() {}

    void start()
    {
      if (handle_) return;
      uintptr_t handle = _beginthreadex(
        0,
        0,
        (unsigned(__stdcall*)(void*))run_thread,
        this,
        0,
        &thread_id_
      );
      handle_ = reinterpret_cast<HANDLE>(handle);
    }

    void join()
    {
      if (!handle_) return;
      WaitForSingleObject(handle_, INFINITE);
    }

    unsigned int thread_id()
    {
      return thread_id_;
    }

  protected:
    virtual void run() = 0;

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
      if (handle_) {
        CloseHandle(handle_);
        handle_ = nullptr;
      }
    }

  };

}