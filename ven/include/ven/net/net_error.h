#pragma once

namespace ven
{

  class NetErrorReceiver
  {
  public:
    virtual void on_error(const char* msg) {
      printf("%s\n", msg);
    }
  };


  class NetErrorHandler
  {
  public:
    NetErrorReceiver* err_rcv_ = nullptr;

    NetErrorReceiver* err_rcv()
    {
      return err_rcv_;
    }

    void set_err_rcv(NetErrorReceiver* err_rcv)
    {
      err_rcv_ = err_rcv;
    }

  protected:
    void net_error()
    {
      printf("net_error\n");
    }

    template <size_t size = 8192>
    void net_error(
      cchar_t* msg,
      cchar_t* file, cchar_t* func, int_t line
    )
    {
      if (!err_rcv_) return;
      char_t buf[size] = { 0, };
      ven::make_str(buf, "%s. %s (%s:%d)", msg, func, file, line);
      err_rcv_->on_error(buf);
    }

    template <size_t size = 8192>
    void net_error(
      cchar_t* msg, int_t err,
      cchar_t* file, cchar_t* func, int_t line
    )
    {
      if (!err_rcv_) return;
      char_t buf[size] = { 0, };
      ven::make_str(buf, "%s[%d]. %s (%s:%d)", msg, err, func, file, line);
      err_rcv_->on_error(buf);
    }

    template <size_t size = 8192>
    void net_error(
      cchar_t* msg, const Addr& addr, int_t err,
      cchar_t* file, cchar_t* func, int_t line
    )
    {
      if (!err_rcv_) return;
      char_t buf[size] = { 0, };
      ven::make_str(
        buf,
        "%s[%d] (%s:%hd). %s (%s:%d)",
        msg, err,
        addr.ip_, addr.port_,
        func, file, line
      );
      err_rcv_->on_error(buf);
    }

    template <size_t size = 8192>
    void net_error(
      cchar_t* msg, const Addr& addr1, const Addr& addr2, int_t err,
      cchar_t* file, cchar_t* func, int_t line
    )
    {
      if (!err_rcv_) return;
      char_t buf[size] = { 0, };
      ven::make_str(
        buf,
        "%s[%d] (%s:%hd, %s:%hd). %s (%s:%d)",
        msg, err,
        addr1.ip_, addr1.port_,
        addr2.ip_, addr2.port_,
        func, file, line
      );
      err_rcv_->on_error(buf);
    }
  };

#define NET_FFL filename(__FILE__), __FUNCTION__, __LINE__

}
