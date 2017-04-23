#include "stdafx.h"

#include <ven/net/net.h>

class Session : public ven::SessionTCP {
public:

  virtual ven::ui32_t receive_buffer_size() override
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

int main()
{
  usage();

  ven::MemPool* mpool = new ven::MemPool;
  mpool->init({
    { 8 * 1024,{ 1000, 100 } }, // 64k 1000°³
  });

  ven::NetConf nconf;
  nconf.mpool_ = mpool;

  ven::Net* net = new ven::Net(nconf);

  if (!net->run_tcp_server<Session>()) {
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
