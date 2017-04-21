#pragma once

#include "../util/thread.h"
#include <queue>

namespace ven {

  enum class OVType {
    None,
    Acpt,
    Conn,
    Disc,
    Send,
    Recv,
  };

  class OV;
  class Actor
  {
  public:
    Actor() {}
    virtual ~Actor() {}

    virtual void on_event(err_t err, OV* ov, ui32_t bytes) = 0;
  };


  class OV : public OVERLAPPED {
  public:
    OVType type_ = OVType::None;
    Actor* actor_ = nullptr;

  public:
    OV() {}
    ~OV() {}

    void set(Actor* actor)
    {
      actor_ = actor;
    }

    void reset()
    {
      Internal = 0;
      InternalHigh = 0;
      Offset = 0;
      OffsetHigh = 0;
      hEvent = 0;
    }
  };


  class AOV : public OV , public Next<AOV> {
  public:
    SessionTCP* sess_;
    addr_buf_t buf_;

  public:
    AOV()
    {
      OV::type_ = OVType::Acpt;
    }

    void reset(SessionTCP* sess)
    {
      OV::reset();
      sess_ = sess;
    }

  };


  class COV : public OV {
  public:
    COV(Actor* actor = nullptr)
    {
      OV::type_ = OVType::Conn;
      OV::actor_ = actor;
    }
  };


  class DOV : public OV {
  public:
    ui32_t err_ = 0;

  public:
    DOV(Actor* actor = nullptr)
    {
      OV::type_ = OVType::Disc;
      OV::actor_ = actor;
    }

    void reset(ui32_t err)
    {
      OV::reset();
      err_ = err;
    }
  };


  class ROV : public OV {
  public:
    ROV(Actor* actor = nullptr)
    {
      OV::type_ = OVType::Recv;
      OV::actor_ = actor;
    }
  };

  class SOV : public OV, public Next<SOV> {
  public:
    BufArray slist_;

  public:
    SOV()
    {
      OV::type_ = OVType::Send;
    }

  };

}
