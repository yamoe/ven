#include "stdafx.h"

#include <ven/net/net.h>

class Session : public ven::SessionUDP {
public:
  virtual void on_recv(ven::Buf& buf, ven::Addr& addr) override
  {
    send(buf, addr);
  }

};


class NetError : public ven::NetErrorReceiver {
public:
  virtual void on_error(const char* msg) {
    printf("%s\n", msg);
  }
};


void usage()
{
  printf(
    "  echo server usage : \n"
    "    listen : 0.0.0.0:1200\n"
    "    enter : print state\n"
    "    esc : exit\n"
    "\n\n"
  );
}

int main()
{
  usage();

  // 메모리 풀
  ven::MemPool* mpool = new ven::MemPool;
  mpool->init({
    { 1 * 1024,{ 1000, 100 } }, // 1k 100개 생성. 부족시 100개 생성
    { 8 * 1024,{ 1000, 100 } }, // 8k 100개
  });

  // Net 객체 생성
  ven::NetConf nconf;
  nconf.thread_cnt_ = 0;
  nconf.mpool_ = mpool;
  nconf.sov_pool_init_size_ = 0;
  nconf.sov_pool_step_size_ = 0;
  nconf.sov_pool_arr_size_ = 0;
  nconf.err_rcv_ = new NetError;

  ven::Net* net = new ven::Net(nconf);

  // 서버 시작
  Session* session = net->run_udp<Session>({ "0.0.0.0", 12000 });
  if (!session) {
    printf("fail run udp server\n");
    return 0;
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
