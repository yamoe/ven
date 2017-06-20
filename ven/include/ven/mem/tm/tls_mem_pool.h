#pragma once

namespace ven {
  namespace tm {

    class TlsMemPool
      : public Singleton<TlsMemPool>
      , public IMemPool
    {
    private:
      Auto<Heap> heap_;
      HeaptList hlist_;
      Tls<Heapt> tls_;

      MemAlloc alloc_;
      uint32_t max_unit_ = 0;

    public:
      TlsMemPool() {}
      virtual ~TlsMemPool() {}

      void init(MemConf conf)
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

      virtual Buf get(uint32_t size) override
      {
        if (size > max_unit_) {
          return new_mem(size);
        }

        Heapt* tm = get_thread_mem();
        Mem* mem = tm->pop(size);

        mem->mpool_ = this;
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

      Mem* new_mem(uint32_t size) {
        alloc_.state().exceed_new_++;

        Mem* mem = new Mem;
        mem->addr_ = static_cast<uint8_t*>(malloc(size));
        mem->unit_ = size;
        mem->mpool_ = this;
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
      TlsMemPool::inst().init(conf);
    }

    static void uninit()
    {
      TlsMemPool::uninst();
    }

    static Buf get(uint32_t size)
    {
      return TlsMemPool::inst().get(size);
    }

    static MemState state()
    {
      return TlsMemPool::inst().state();
    }

    static TlsMemPool& inst()
    {
      return TlsMemPool::inst();
    }

  }
}
