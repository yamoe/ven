#pragma once

namespace ven {
  namespace tm {

    class UnitMemConf
    {
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
  }
}