#pragma once

namespace ven {

  template <int size = 8192>
  class CharArray
  {
  private:
    char arr_[size] = { 0 };
    int pos_ = 0;

  public:
    CharArray() {}

    operator const void*() { return arr_; }

    operator const char*() { return arr_; }

    operator std::string() { return arr_; }

    int len() { return pos_; }

    CharArray& reset()
    {
      pos_ = 0;
      return *this;
    }

    CharArray& add(char* format, ...)
    {
      if (pos_ == -1) {
        return *this;
      }

      va_list list;
      va_start(list, format);
      int ret = vsnprintf_s(arr_ + pos_, size - pos_, size - pos_ - 1, format, list);
      va_end(list);
      
      if (ret == -1) {
        pos_ = -1;
      } else {
        pos_ += ret;
      }
      return *this;
    }

    char* ch()
    {
      return arr_;
    }

  };

}
