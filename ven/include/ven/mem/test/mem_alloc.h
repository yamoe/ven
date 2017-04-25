#pragma once

namespace ven {
  namespace test {

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

      byte_t* new_mem_for_heap(MemConf& conf, MemMap& map)
      {
        size_t size = 0;
        for (auto& kv : conf) {
          size += mem_size(kv.first, kv.second.cnt_);
        }

        byte_t* p = static_cast<byte_t*>(malloc(size));

        byte_t* b = p;
        for (auto& kv : conf) {
          unit_t unit = kv.first;
          ui32_t cnt = kv.second.cnt_;

          MemList ml;
          b = set_mem_list(b, unit, cnt, ml);
          map.emplace(unit, ml);

          state_.units_[unit].total_ += cnt;
        }
        return p;
      }

      byte_t* new_mem(unit_t unit, ui32_t cnt, MemList& ml)
      {
        size_t size = mem_size(unit, cnt);
        byte_t* p = static_cast<byte_t*>(malloc(size));

        set_mem_list(p, unit, cnt, ml);

        state_.units_[unit].total_ += cnt;
        return p;
      }


    private:
      byte_t* set_mem_list(byte_t* p, unit_t unit, ui32_t cnt, MemList& ml)
      {
        for (ui32_t i = 0; i < cnt; ++i) {
          Mem* m = new(p) Mem;
          p += mem_size_;

          m->addr_ = p;
          m->unit_ = unit;
          p += unit;

          ml.push(m);
        }
        return p;
      }

      size_t mem_size(unit_t unit, ui32_t cnt)
      {
        return (
          (mem_size_ * cnt) + // Mem 
          (unit * cnt)  // Mem 객체에서 사용할 메모리
          );
      }

    };

  }
}