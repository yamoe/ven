#pragma once

#include "../../util/inc.h"

#include "type.h"
#include "objs.h"
#include "mem.h"
#include "buf.h"
#include "mem_alloc.h"
#include "heap.h"
#include "heapt.h"
#include "heapt_list.h"

namespace ven {
  namespace test {

    class Memory
      : public Singleton<Memory>
      , public IMemory
    {
    private:
      Auto<Heap> heap_;
      HeaptList hlist_;
      Tls<Heapt> tls_;

      MemAlloc alloc_;
      unit_t max_unit_ = 0;

    public:
      Memory() {}
      ~Memory() {}

      virtual void init(MemConf conf) override
      {
        if (heap_) return;

        if (conf.empty()) {
          conf = {
            { 1 * 1024,  { 100, 10, 5 } }, // 1k 100개, 10 개씩 Heap에서 가져옴, 스레드에 10개가 반납되면 Heap으로 모두 반납.
            { 2 * 1024,  { 100, 10, 5 } }, // 2k 100개
            { 4 * 1024,  { 100, 10, 5 } }, // 4k 100개
            { 8 * 1024,  { 100, 10, 5 } }, // 8k 100개
          };
        }

        max_unit_ = conf.rbegin()->first;
        heap_ = new Heap(alloc_, conf);
      }

      virtual Buf get(unit_t size) override
      {
        if (size > max_unit_) {
          return new_mem(size);
        }

        Heapt* tm = get_thread_mem();
        Mem* mem = tm->pop(size);

        mem->memory_ = this;
        mem->ref_ = 0;

        return mem;
      }

      virtual void ret(Mem* mem) override
      {
        if (mem->unit_ > max_unit_) {
          del_mem(mem);
          return;
        }

        Heapt* tm = get_thread_mem();
        tm->push(mem);
      }

      virtual MemState state() override
      {
        MemState s = alloc_.state();

        heap_->state(s);
        hlist_.state(s);

        for (auto& kv : s.units_) {
          auto& us = kv.second;
          us.use_ = us.total_ - us.wait_ - us.free_;
        }
        return s;
      }

    private:

      Mem* new_mem(unit_t size) {
        alloc_.state().exceed_new_++;

        Mem* mem = new Mem;
        mem->addr_ = static_cast<byte_t*>(malloc(size));
        mem->unit_ = size;
        mem->memory_ = this;
        return mem;
      }

      void del_mem(Mem* mem) {
        alloc_.state().exceed_del_++;
        free(mem->addr_);
        delete mem;
      }

      Heapt* get_thread_mem()
      {
        Heapt* tm = tls_.get();
        if (!tm) {
          tm = new Heapt(alloc_, *heap_);
          hlist_.add(tm);
          tls_.set(tm);
        }
        return tm;
      }
    };


    static void init(MemConf conf = MemConf())
    {
      Memory::inst().init(conf);
    }

    static void uninit()
    {
      Memory::uninst();
    }

    static Buf get(unit_t size)
    {
      return Memory::inst().get(size);
    }

    static MemState state()
    {
      return Memory::inst().state();
    }

    static Memory& inst()
    {
      return Memory::inst();
    }

  }
}
