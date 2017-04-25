#include "stdafx.h"

#include <ven/net/net.h>

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
  ven::ui32_t seq_ = 0;
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
    ar << seq_;
  }
};


class EchoPacket
  : public PacketHeader
{
public:
  std::string str_;

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

class Session : public ven::SessionUDP {
public:
  virtual void on_recv(ven::Buf& buf, ven::Addr& addr) override
  {
    PacketType type = buf.get<PacketType>(4);

    if (type == PacketType::EchoPacket) {
      EchoPacket packet;
      packet.make_from(buf);
      printf("recv : %s\n", packet.str_.c_str());
    }
    else {
      printf("recv unknown packet");
    }
  }
};


void usage()
{
  printf(
    "  echo client usage : \n"
    "    connect/send : 127.0.0.1:12000\n"
    "    s : send\n"
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

  Session* session = net->run_udp<Session>({},  { "127.0.0.1", 12000 });

  while (true) {
    Sleep(10);

    if (_kbhit() > 0) {
      int ch = _getch();

      if (ch == 27) { // ESC
        printf("exit\n");
        break;
      }
      
      if (ch == 83 || ch == 115) { // S or s
        printf("send\n");
        EchoPacket packet;
        packet.str_ = "echo packet";
        session->send(packet, { "127.0.0.1", 12000 });
      }

      if (ch == 13) { // ENTER
        ven::NetState s = net->state();
        s.print();
      }

    }
  }

  net->stop_udp(session);
  delete net;

  return 0;
}
