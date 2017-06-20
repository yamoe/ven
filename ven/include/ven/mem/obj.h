#pragma once

namespace ven {

  class Mem;
  class Buf;
  class MemState;
  class IMemPool {
  public:
    IMemPool() {}
    virtual ~IMemPool() {}
  public:
    virtual Buf get(uint32_t size) = 0;
    virtual void ret(Mem* mem) = 0;
    virtual MemState state() = 0;
  };


  class MemConf {
  public:
    uint32_t init_cnt_ = 0;
    uint32_t step_cnt_ = 0;

  public:
    MemConf(uint32_t init_cnt, uint32_t step_cnt)
      : init_cnt_(init_cnt)
      , step_cnt_(step_cnt)
    {}
  };

  typedef std::map<uint32_t, MemConf> MemPoolConf;


  class UnitMemState
  {
  public:
    uint32_t total_ = 0;
    uint32_t wait_ = 0; //TlsMemPool 사용시만 쓰임
    uint32_t free_ = 0;
    uint32_t use_ = 0;
  };

  class MemState
  {
  public:
    typedef std::map<uint32_t, UnitMemState> Units;
    Units units_;

    uint32_t exceed_new_ = 0;
    uint32_t exceed_del_ = 0;

  public:
    void print()
    {
      std::string s = make_str(
        "--------------------------------------------\n"
        "  exceed new : %u, del: %u\n", exceed_new_, exceed_del_
      );

      for (auto& kv : units_) {
        uint32_t unit = kv.first;
        auto& us = kv.second;

        s += make_str(
          "  %u - total : %u, wait : %u, free : %u, use : %u\n",
          unit, us.total_, us.wait_, us.free_, us.use_
        );
      }
      printf("%s", s.c_str());
    }
  };

}