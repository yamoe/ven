#pragma once

namespace ven {

  class MemPool
    : public IMemPool
  {
  private:
    //typedef LFStack<Mem> MemList;
    typedef SListLocker<Mem> MemList;

    class Mems
    {
    public:
      MemList free_;

      std::atomic<ui32_t> total_ = 0;
      ui32_t init_cnt_ = 0;
      ui32_t step_cnt_ = 0;  //증가량
      ui32_t del_cnt_ = 0;  //증가량
    };
    typedef std::map<unit_t, Mems> Map;

    SLock lock_;
    Map map_;
    unit_t max_unit_ = 0;

    std::atomic<ui32_t> exceed_new_ = 0;
    std::atomic<ui32_t> exceed_del_ = 0;

  public:
    MemPool() {}
    virtual ~MemPool() {}

    void init(MemPoolConf conf = MemPoolConf())
    {
      if (conf.empty()) {
        conf = {
          { 1 * 1024, { 100, 100 } }, // 1k 100개 생성. 부족시 100개 생성
          { 2 * 1024, { 100, 100 } }, // 2k 100개
          { 4 * 1024, { 100, 100 } }, // 4k 100개
          { 8 * 1024, { 1000, 100 } }, // 8k 100개
        };
      }

      max_unit_ = conf.rbegin()->first;

      for (auto& kv : conf) {
        unit_t unit = kv.first;
        MemConf& mc = kv.second;

        Mems& mems = map_[unit];

        mems.total_ = mc.init_cnt_;
        mems.init_cnt_ = mc.init_cnt_;
        mems.step_cnt_ = mc.step_cnt_;
        mems.del_cnt_ = mc.init_cnt_ * 2;

        for (ui32_t i = 0; i < mc.init_cnt_; ++i) {
          mems.free_.push(new_mem(unit));
        }
      }
    }

    virtual Buf get(unit_t size) override
    {
      if (size > max_unit_)
      {
        return new_mem(size);
      }

      auto& kv = map_.lower_bound(size);
      unit_t unit = kv->first;
      Mems& mems = kv->second;

      Mem* mem = mems.free_.pop();
      if (!mem) {
        VEN_LOCKER(mems.free_.lock());
        mem = mems.free_.pop();
        if (mem) return mem;

        inc_mem(unit, mems);
        return mems.free_.pop();
      }
      return mem;
    }

    virtual void ret(Mem* mem) override
    {
      if (mem->unit_ > max_unit_)
      {
        del_mem(mem);
        return;
      }

      Mems& mems = map_[mem->unit_];

      //if (mems.free_.cnt() >= mems.del_cnt_) {
      //  mems.total_--;
      //  del_mem(mem);
      //  return;
      //}

      mems.free_.push(mem);
    }
    
    virtual MemState state() override
    {
      MemState s;
      s.exceed_new_ = exceed_new_;
      s.exceed_del_ = exceed_del_;

      for (auto& kv : map_) {
        Mems& mems = kv.second;
        UnitMemState& ums = s.units_[kv.first];
        ums.total_ = mems.total_;
        ums.free_ = mems.free_.cnt();
        ums.use_ = ums.total_ - ums.free_;
      }
      return s;
    }

  private:
    Mem* new_mem(unit_t unit) {
      Mem* mem = new Mem;
      mem->addr_ = static_cast<byte_t*>(malloc(unit));
      mem->unit_ = unit;
      mem->mpool_ = this;
      return mem;
    }

    void del_mem(Mem* mem) {
      free(mem->addr_);
      delete mem;
    }

    void inc_mem(unit_t unit, Mems& mems)
    {
      ui32_t cnt = mems.step_cnt_;
      mems.total_ += cnt;

      SList<Mem> slist;
      for (ui32_t i = 0; i < cnt; ++i) {
        slist.push(new_mem(unit));
      }
      mems.free_.push(slist);
    }

  };

}
