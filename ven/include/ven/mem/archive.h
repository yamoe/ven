#pragma once

namespace ven {

  class Archive
  {
  public:
    enum Mode {
      Size,
      Write,
      Read,
    };

  private:
    Mode mode_;
    Buf buf_;
    ui32_t pos_ = 0;

  public:
    Archive(Mode mode = Read)
      : mode_(mode)
    {}

    ~Archive() {}

    void set(Buf& buf, Mode mode = Write)
    {
      buf_ = buf;
      mode_ = mode;
      pos_ = 0;
    }

    Buf buf()
    {
      return buf_;
    }

    ui32_t size()
    {
      return pos_;
    }

    template <class T>
    Archive& operator<<(T& v)
    {
      switch (mode_) {
      case Size:
        pos_ += Sizer::size(v);
        break;
      case Write:
        pos_ += buf_.set(v, pos_);
        break;
      case Read:
        pos_ += buf_.get(v, pos_);
        break;
      }
      return *this;
    }
  };

}
