#pragma once

namespace ven {
  namespace test {

    class HeaptList {
    private:
      SLock lock_;
      std::vector<Heapt*> list_;

    public:
      HeaptList() {}
      ~HeaptList()
      {
        VEN_LOCKER(lock_);
        for (auto t : list_) {
          delete t;
        }
      }

      void add(Heapt* v)
      {
        VEN_LOCKER(lock_);
        list_.push_back(v);
      }

      void state(MemState& s)
      {
        VEN_LOCKER(lock_);
        for (auto v : list_) {
          v->state(s);
        }
      }

    };

  }
}