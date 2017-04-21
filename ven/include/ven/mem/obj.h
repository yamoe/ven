#pragma once

namespace ven {

  typedef unsigned __int32 unit_t;

  class Mem;
  class Buf;
  class IMemPool {
  public:
    virtual Buf get(unit_t size) = 0;
    virtual void ret(Mem* mem) = 0;
  };


  class MemConf {
  public:
    ui32_t init_cnt_ = 0;
    ui32_t step_cnt_ = 0;

  public:
    MemConf(ui32_t init_cnt, ui32_t step_cnt)
      : init_cnt_(init_cnt)
      , step_cnt_(step_cnt)
    {}
  };

  typedef std::map<unit_t, MemConf> MemPoolConf;



  class UnitMemState {
  public:
    ui32_t total_ = 0;
    ui32_t free_ = 0;
    ui32_t use_ = 0;
  };


  class MemState {
  public:
    typedef std::map<unit_t, UnitMemState> Units;
    Units units_;

    ui32_t exceed_new_ = 0;
    ui32_t exceed_del_ = 0;

  public:
    void print()
    {
      std::string s = make_str(
        "--------------------------------------------\n"
        "  exceed new : %u, del: %u\n", exceed_new_, exceed_del_
      );

      for (auto& kv : units_) {
        unit_t unit = kv.first;
        auto& us = kv.second;

        s += make_str(
          "  %u - total : %u, free : %u, use : %u\n",
          unit, us.total_, us.free_, us.use_
        );
      }
      printf("%s", s.c_str());
    }
  };

}