#pragma once

namespace ven {
  namespace tm {

    class Heapt {
    private:

      class HeapMem {
      public:
        unit_t unit_;
        UnitMemConf conf_;
        MemList wait_;
        MemList free_;
        MemListLocker& heap_wait_;

      public:
        HeapMem(unit_t u, UnitMemConf& uc, MemListLocker* lml)
          : unit_(u)
          , conf_(uc)
          , heap_wait_(*lml)
        {}
      };

      typedef std::map<unit_t, HeapMem*> Map;

      MemAlloc& alloc_;
      std::vector<byte_t*> p_;

      Map map_;
      int_t tid_;

    public:
      Heapt(MemAlloc& alloc, Heap& heap)
        : alloc_(alloc)
        //, tid_(::GetCurrentThreadId())
      {
        MemConf& conf = heap.conf();
        for (auto& kv : conf) {
          unit_t unit = kv.first;
          map_[unit] = new HeapMem(
            unit,
            kv.second,
            heap.get(unit)
          );
        }
      }

      ~Heapt()
      {
        for (auto& it : p_) free(it);
        for (auto& it : map_) delete it.second;
      }

      void state(MemState& s)
      {
        for (auto& kv : map_) {
          unit_t unit = kv.first;
          HeapMem& hm = *kv.second;

          auto& us = s.units_[unit];
          us.free_ += hm.free_.cnt();
          us.wait_ += hm.wait_.cnt();
        }
      }

      Mem* pop(unit_t size)
      {
        HeapMem& hm = get(size);
        Mem* mem = pop(hm);
        //mem->tid_ = tid_;
        return mem;
      }

      void push(Mem* mem)
      {
        //HeapMem& hm = get(mem->unit_);
        HeapMem& hm = *map_[mem->unit_];

        //if (mem->tid_ == tid_) {
        //  hm.wait_.push(mem);
        //  return;
        //}

        hm.free_.push(mem);
        if (hm.free_.cnt() >= hm.conf_.push_cnt_) {
          hm.heap_wait_.push(hm.free_);
        }
      }

    private:
      HeapMem& get(unit_t size)
      {
        return *map_.lower_bound(size)->second;
      }

      Mem* pop(HeapMem& hm)
      {
        Mem* mem = nullptr;

        mem = pop_from_thread(hm);
        if (mem) return mem;

        mem = pop_from_main(hm);
        if (mem) return mem;

        mem = pop_from_new(hm);
        if (mem) return mem;

        return mem;
      }

      Mem* pop_from_thread(HeapMem& hm)
      {
        Mem* mem = hm.wait_.pop();
        if (mem) return mem;

        return hm.free_.pop();
      }

      Mem* pop_from_main(HeapMem& hm)
      {
        MemList ml;
        hm.heap_wait_.pop(hm.conf_.pop_cnt_, ml);
        if (ml) {
          Mem* mem = ml.pop();
          hm.wait_.push(ml);
          return mem;
        }
        return nullptr;
      }

      Mem* pop_from_new(HeapMem& hm)
      {
        MemList ml;
        byte_t* p = alloc_.new_mem(hm.unit_, hm.conf_.cnt_, ml);
        p_.push_back(p);

        ui32_t cnt = ml.cnt();
        if (cnt > hm.conf_.pop_cnt_) {
          ui32_t ret_cnt = cnt - hm.conf_.pop_cnt_;

          MemList heap_ml;
          ml.pop(ret_cnt, heap_ml);
          hm.heap_wait_.push(heap_ml);
        }

        Mem* mem = ml.pop();
        hm.wait_.push(ml);
        return mem;
      }

    };

  }
}