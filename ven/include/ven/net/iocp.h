#pragma once

namespace ven {

  class IOCP
    : public NetErrorHandler
  {
  private:
    HANDLE handle_ = NULL;

  public:
    IOCP() {}

    ~IOCP() {}

    void init()
    {
      handle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
      if (!handle_) {
        net_error("CreateIoCompletionPort", GetLastError(), NET_FFL);
      }
    }

    void uninit()
    {
      if (handle_) {
        CloseHandle(handle_);
        handle_ = NULL;
      }
    }

    bool regist(HANDLE handle)
    {
      SOCKET sock = 0;
      if (!CreateIoCompletionPort(
        handle, handle_, 0, 0
      )) {
        return false;
      }
      return true;
    }

    bool get(uint32_t& bytes, OV*& ov, DWORD timeout = INFINITE)
    {
      ULONG_PTR key = 0;
      return (GetQueuedCompletionStatus(
        handle_,
        reinterpret_cast<LPDWORD>(&bytes),
        reinterpret_cast<PULONG_PTR>(&key),
        reinterpret_cast<LPOVERLAPPED*>(&ov),
        timeout
      ) == TRUE);
    }

    operator bool() const
    {
      return (handle_ != NULL);
    }
    /*
    void post()
    {
      DWORD bytes = 0;
      ULONG_PTR key = 0;
      OVERLAPPED* overlapped = NULL;
      if (!PostQueuedCompletionStatus(handle_, bytes, key, overlapped)) {
        net_error("PostQueuedCompletionStatus", GetLastError(), NET_FFL);
      }
    }
    */
  };

}
