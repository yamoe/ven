#include "stdafx.h"

#include <ven/net/inc.h>

class Session : public ven::SessionTCP {
public:

  virtual uint32_t receive_buffer_size() override
  {
    return (1024 * 8);
  }

  virtual void on_recv(ven::Buf& buf) override
  {
    send(buf);
  }

};

void usage()
{
  printf(
    "  echo server usage : \n"
    "    listen : 0.0.0.0:12001\n"
    "    enter : print state\n"
    "    esc : exit\n"
    "\n\n"
  );
}

// 로거
VENL_DECL(SVR);

class NetErrorReceiver
  : public ven::NetErrorReceiver
{
public:
  virtual void on_error(const char* msg) {
    VENL_E(SVR)("net wrror : %s", msg);
  }
};

int main()
{
  /*
    로거 초기화
    매일 18시 00분 파일 교체
    레벨 : debug 이상
    콘솔 출력 가능시 출력
    비동기 기록 false
  */
  VENL_INIT(SVR)(
    new ven::log::DailyLog(18, 30, ven::log::LogLevel::Debug, true, false)
  );
  VENL_I(SVR)("ven echo tcp server");

  // minidump 설정
  ven::CrashDump::install();

  // 정보 출력
  usage();


  // 세션 receive_buffer_size 에 맞춰서 메모리 풀 설정
  ven::IMemPool* mpool = nullptr;
  if (true) {
    ven::MemPool* mp = new ven::MemPool;
    mp->init({
      { 8 * 1024,{ 1000, 100 } }, // 8k 1000개, 100개씩 증가
    });
    mpool = mp;
  } else {
    // TLS 를 사용하는 방식
    ven::tm::TlsMemPool* mp = new ven::tm::TlsMemPool;
    mp->init({
      { 8 * 1024,{ 1000, 100, 10 } }, // 8k 1000개,  100 개씩 Heap에서 가져옴, 스레드에 10개가 반납되면 Heap으로 모두 반납. 모자란 경우 초기 값인 1000 개 만큼 증가
    });
    mpool = mp;
  }

  // Net 생성
  ven::NetConf nconf;
  nconf.mpool_ = mpool;
  nconf.err_rcv_ = new NetErrorReceiver;

  ven::Net* net = new ven::Net(nconf);

  // 서버 시작
  ven::ServerConf sconf;
  sconf.session_pre_cnt_ = 3000;
  if (!net->run_tcp_server<Session>(sconf)) {
    printf("fail run server\n");
  }

  while (true) {
    Sleep(10);

    if (_kbhit() > 0) {
      int ch = _getch();

      if (ch == 27) { //ESC
        printf("exit\n");
        break;
      }

      if (ch == 13) { // ENTER
        ven::NetState s = net->state();
        s.print();
      }
    }
  }

  delete net;
  delete mpool;

  return 0;
}
