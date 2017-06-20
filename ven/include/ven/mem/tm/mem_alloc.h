#pragma once

namespace ven {
  namespace tm {

    class MemAlloc {
    private:
      MemState state_;
      size_t mem_size_ = 0;

    public:
      MemAlloc()
        : mem_size_(sizeof(Mem))
      {}

      ~MemAlloc() {}

      MemState& state()
      {
        return state_;
      }

      uint8_t* new_mem_for_heap(MemConf& conf, MemMap& map)
      {
        size_t size = 0;
        for (auto& kv : conf) {
          size += mem_size(kv.first, kv.second.cnt_);
        }

        uint8_t* p = static_cast<uint8_t*>(malloc(size));

        uint8_t* b = p;
        for (auto& kv : conf) {
          uint32_t unit = kv.first;
          uint32_t cnt = kv.second.cnt_;

          MemList ml;
          b = set_mem_list(b, unit, cnt, ml);
          map.emplace(unit, ml);

          state_.units_[unit].total_ += cnt;
        }
        return p;
      }

      uint8_t* new_mem(uint32_t unit, uint32_t cnt, MemList& ml)
      {
        size_t size = mem_size(unit, cnt);
        uint8_t* p = static_cast<uint8_t*>(malloc(size));

        set_mem_list(p, unit, cnt, ml);

        state_.units_[unit].total_ += cnt;
        return p;
      }


    private:
      uint8_t* set_mem_list(uint8_t* p, uint32_t unit, uint32_t cnt, MemList& ml)
      {
        for (uint32_t i = 0; i < cnt; ++i) {
          Mem* m = new(p) Mem;
          p += mem_size_;

          m->addr_ = p;
          m->unit_ = unit;
          p += unit;

          ml.push(m);
        }
        return p;
      }

      size_t mem_size(uint32_t unit, uint32_t cnt)
      {
        return (
          (mem_size_ * cnt) + // Mem 
          (unit * cnt)  // Mem 객체에서 사용할 메모리
          );
      }

    };

  }
}