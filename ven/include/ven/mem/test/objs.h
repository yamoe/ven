#pragma once

namespace ven {
  namespace test {

    class UnitMemConf {
    public:
      ui32_t cnt_ = 0;
      ui32_t pop_cnt_ = 0;
      ui32_t push_cnt_ = 0;

    public:
      UnitMemConf(ui32_t cnt, ui32_t pop_cnt, ui32_t push_cnt)
        : cnt_(cnt)
        , pop_cnt_(pop_cnt)
        , push_cnt_(push_cnt)
      {}
    };


    typedef std::map<unit_t, UnitMemConf> MemConf;


    class Buf;
    class MemState;
    class IMemory {
    public:
      virtual void init(MemConf conf) = 0;
      virtual Buf get(unit_t size) = 0;
      virtual void ret(Mem* mem) = 0;
      virtual MemState state() = 0;
    };


    class UnitMemState {
    public:
      ui32_t total_ = 0;
      ui32_t wait_ = 0;
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
            "  %u - total : %u, wait : %u, free : %u, use : %u\n",
            unit, us.total_, us.wait_, us.free_, us.use_
          );
        }
        printf("%s", s.c_str());
      }
    };

  }
}