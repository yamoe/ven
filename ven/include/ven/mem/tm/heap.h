#pragma once

namespace ven {
  namespace tm {

    class Heap {
    private:
      typedef std::map<uint32_t, MemListLocker*> Map;

      MemAlloc& alloc_;
      MemConf conf_;

      Map map_;
      uint8_t* p_ = nullptr;

    public:
      Heap(MemAlloc& alloc, MemConf& conf)
        : alloc_(alloc)
        , conf_(conf)
      {
        MemMap m;
        p_ = alloc_.new_mem_for_heap(conf, m);

        for (auto& kv : conf) {
          map_[kv.first] = new MemListLocker(m[kv.first]);
        }
      }

      ~Heap()
      {
        free(p_);
        p_ = nullptr;
      }

      MemListLocker* get(uint32_t unit)
      {
        return map_[unit];
      }

      MemConf& conf()
      {
        return conf_;
      }

      void state(MemState& s)
      {
        for (auto& kv : map_) {
          uint32_t unit = kv.first;
          MemListLocker* lml = kv.second;

          s.units_[unit].wait_ += lml->cnt();
        }
      }

    };

  }
}