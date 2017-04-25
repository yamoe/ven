#pragma once

namespace ven {

  class BufConv
  {
  public:
    BufConv() {}

    virtual ~BufConv() {}

    // 패킷을 Buf에 담을 패킷 길이 저장
    virtual void set_size(ui32_t size) = 0;

    // 패킷, Buf간 복사시 수행
    virtual void serialize(Archive& ar) = 0;

    Buf make_buf(IMemPool& mpool)
    {
      // 크기 계산
      Archive ar(Archive::Size);
      serialize(ar);

      ui32_t size = ar.size();
      set_size(size);
      if (size == 0) {
        return Buf();
      }

      // 버퍼 할당
      Buf buf = mpool.get(size);
      if (!buf) return buf;
      buf.len_ = size;

      // 버퍼 패킷  쓰기
      ar.set(buf, Archive::Write);
      serialize(ar);
      return ar.buf();
    }

    void make_from(Buf& buf)
    {
      Archive ar;
      ar.set(buf, Archive::Read);
      serialize(ar);
    }

  };
}