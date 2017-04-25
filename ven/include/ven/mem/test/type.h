#pragma once

namespace ven {
  namespace test {

    typedef unsigned __int32 unit_t;  // 메모리 단위
    typedef unsigned long tid_t;  // thread id

    class Mem;
    template class SList<Mem>;
    typedef SList<Mem> MemList;
    typedef std::map<unit_t, MemList> MemMap;
    typedef SListLocker<Mem> MemListLocker;

  }
}
