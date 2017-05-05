#include "stdafx.h"

#include <ven/net/inc.h>

enum class PacketType
{
  None,
  EchoPacket,
};


class PacketHeader
  : public ven::BufConv
{
public:
  ven::ui32_t len_ = 0;
  PacketType type_;

public:
  PacketHeader(PacketType type)
    : type_(type)
  {}

  virtual void set_size(ven::ui32_t size) override
  {
    len_ = size;
  }

  virtual void serialize(ven::Archive& ar) override
  {
    ar << len_;
    ar << type_;
  }
};


class EchoPacket
  : public PacketHeader
{
public:
  std::string str_ = "echo packet";

public:
  EchoPacket()
    : PacketHeader(PacketType::EchoPacket)
  {}

  virtual void serialize(ven::Archive& ar) override
  {
    PacketHeader::serialize(ar);
    ar << str_;
  }
};


class Session : public ven::SessionTCP {
public:

  virtual void on_conn() override
  {
    printf("connected\n");
  }

  virtual void on_disc(ven::ui32_t err) override
  {
    printf("disconnected err : %u\n", err);
  }

  virtual void on_recv(ven::Buf& buf) override
  {
    PacketType type = buf.get<PacketType>(4);

    if (type == PacketType::EchoPacket) {
      EchoPacket packet;
      packet.make_from(buf);
      printf("recv packet : %s\n", packet.str_.c_str());
    } else {
      printf("recv unknown packet");
    }
  }
};

void usage()
{
  printf(
    "  echo client usage : \n"
    "    connect : 127.0.0.1:12001\n"
    "    c : connect\n"
    "    s : send\n"
    "    d : disconnect\n"
    "    enter : print state\n"
    "    esc : exit\n"
    "\n\n"
  );
}

int main()
{
  ven::CrashDump::install();

  usage();

  ven::Net* net = new ven::Net;
  Session* session = net->run_tcp_client<Session>({ "127.0.0.1", 12001 });

  while (true) {
    Sleep(10);

    if (_kbhit() > 0) {
      int ch = _getch();

      if (ch == 27) { // ESC
        printf("exit\n");
        break;
      }

      if (ch == 99 || ch == 67) { // C or c
        printf("connect\n");
        if (!session->connect()) {
          printf("  fail connect\n");
        }
      }

      if (ch == 100 || ch == 68) { // D or d
        printf("disconnect\n");
        session->disconnect();
      }

      if (ch == 83 || ch == 115) { // S or s
        printf("send\n");
        EchoPacket packet;
        if (!session->send(packet)) {
          printf("  fail send\n");
        }
      }

      if (ch == 13) { // ENTER
        ven::NetState s = net->state();
        s.print();
      }

    }
  }

  net->stop_tcp_client(session);
  delete net;

  return 0;
}
