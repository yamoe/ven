#pragma once

namespace ven {
  namespace test {

    class Heap {
    private:
      typedef std::map<unit_t, MemListLocker*> Map;

      MemAlloc& alloc_;
      MemConf conf_;

      Map map_;
      byte_t* p_ = nullptr;

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

      MemListLocker* get(unit_t unit)
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
          unit_t unit = kv.first;
          MemListLocker* lml = kv.second;

          s.units_[unit].wait_ += lml->cnt();
        }
      }

    };

  }
}