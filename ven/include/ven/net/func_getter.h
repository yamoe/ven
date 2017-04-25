#pragma once


namespace ven {

  class FuncGetter
  {
  public:
    static LPFN_ACCEPTEX acceptex(SOCKET s)
    {
      return get_func<LPFN_ACCEPTEX>(s, WSAID_ACCEPTEX);
    }

    static LPFN_CONNECTEX connectex(SOCKET s)
    {
      return get_func<LPFN_CONNECTEX>(s, WSAID_CONNECTEX);
    }

    static LPFN_DISCONNECTEX disconnectex(SOCKET s)
    {
      return get_func<LPFN_DISCONNECTEX>(s, WSAID_DISCONNECTEX);
    }

    static LPFN_GETACCEPTEXSOCKADDRS getacceptexsockaddrs(SOCKET s)
    {
      return get_func<LPFN_GETACCEPTEXSOCKADDRS>(s, WSAID_GETACCEPTEXSOCKADDRS);
    }

  private:
    template <class Func>
    static Func get_func(SOCKET s, GUID wsaid)
    {
      Func func = nullptr;
      DWORD bytes = 0;
      BOOL ret = WSAIoctl(
        s, SIO_GET_EXTENSION_FUNCTION_POINTER,
        &wsaid, sizeof(wsaid),
        &func, sizeof(func),
        &bytes, NULL, NULL);
      if (ret == SOCKET_ERROR) {
        return nullptr;
      }
      return func;
    }

  };

}
