#pragma once

namespace ven {

  class ISessionTCP
  {
  public:

    // 수신 패킷 사이즈
    virtual bool get_packet_size(byte_t* buf, const ui32_t len, ui32_t& packet_size) = 0;

    // 이상한 패킷 사이즈인 경우 연결 종료 수행
    virtual bool is_valid_packet_size(ui32_t size) = 0;

    // 패킷의 헤더 사이즈(bytes)
    virtual ui32_t header_size() = 0;

    // 수신 버퍼 크기
    virtual ui32_t receive_buffer_size() = 0;

    // 연결
    virtual void on_conn() = 0;

    // 연결 종료
    virtual void on_disc(ui32_t err) = 0;

    // 수신
    virtual void on_recv(ven::Buf& buf) = 0;
  };


  class ISessionUDP
  {
  public:

    /*
    수신하려는 udp 패킷 크기보다 recv 걸리는 버퍼 크기가 작으면
    GQCS recv 에서 ERROR_NO_DATA(232) 발생하므로
    사용하는 UDP 패킷의 최대 크기를 잡아주어야 함
    */
    virtual ui32_t recv_buf_remaining_size_to_exchange() = 0;

    // 수신 버퍼 크기
    virtual ui32_t receive_buffer_size() = 0;

    // 수신
    virtual void on_recv(Buf& buf, Addr& addr) = 0;

  };


  class SessionTCP;
  class ISessionPool
  {
  public:
    ISessionPool() {}
    ~ISessionPool() {}

    virtual SessionTCP* pop() = 0;
    virtual void push(SessionTCP* s) = 0;
  };


  class IAcceptor
  {
  public:
    IAcceptor() {}
    ~IAcceptor() {}

    virtual void disconnected(SessionTCP* s) = 0;
  };


  class SvrState;
  class IServer
  {
  public:
    IServer() {}
    virtual ~IServer() {}

    virtual bool run() = 0;
    virtual void stop() = 0;
    virtual void state(SvrState& s) = 0;
  };

}
