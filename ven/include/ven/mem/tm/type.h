#pragma once

namespace ven {

  class Mem;

  namespace tm {
    template class SList<Mem>;
    typedef SList<Mem> MemList;
    typedef std::map<unit_t, MemList> MemMap;
    typedef SListLocker<Mem> MemListLocker;

  }
}
