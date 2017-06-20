#pragma once

namespace ven {

  class NetErrorReceiver;
  class NetConf
  {
  public:
    uint32_t thread_cnt_ = 0; // 0인 경우 (cpu core * 2) + 1
    IMemPool* mpool_ = nullptr;  // nullptr인 경우 Net 객체안에서 생성

    uint32_t sov_pool_init_size_ = 300; // send overrapped 객체 생성 수
    uint32_t sov_pool_step_size_ = 100; // send overrapped 객체 증가 수(부족 시)
    uint32_t sov_pool_arr_size_ = 1000; // send overrapped 객체당 최대 모아보낼 send 수

    NetErrorReceiver* err_rcv_ = nullptr; // 에러 수신용 객체
  };


  class ServerConf
  {
  public:
    Addr addr_ = { "0.0.0.0", 12001 };  // listen ip,port
    uint32_t accept_cnt_ = 200;  // accept 걸어둘 세션 수
    uint32_t session_pre_cnt_ = 200;  // 미리 생성할 세션 수
    uint32_t session_inc_cnt_ = 100;  // 세션 증가 단위
    uint32_t session_max_cnt_ = 20000;  // 최대 세션 수
    void* session_user_data_ = nullptr; // 세션 객체에 전달할 사용자 데이터 (user_data() 메소드로 접근가능)
  };

  class IOCP;
  class SOVPool;
  class NetData
  {
  public:
    IOCP* iocp_ = nullptr;
    IMemPool* mpool_ = nullptr;
    SOVPool* sov_pool_ = nullptr;
    IAcceptor* acpt_ = nullptr;
  };


  class UserData
  {
  private:
    void* user_data_ = nullptr;

  protected:
    void* user_data()
    {
      return user_data_;
    }

    void set_user_data(void* user_data)
    {
      user_data_ = user_data;
    }
  };

}
