#pragma once

namespace ven {
  namespace tm {

    class UnitMemConf
    {
    public:
      uint32_t cnt_ = 0;
      uint32_t pop_cnt_ = 0;
      uint32_t push_cnt_ = 0;

    public:
      UnitMemConf(uint32_t cnt, uint32_t pop_cnt, uint32_t push_cnt)
        : cnt_(cnt)
        , pop_cnt_(pop_cnt)
        , push_cnt_(push_cnt)
      {}
    };


    typedef std::map<uint32_t, UnitMemConf> MemConf;
  }
}