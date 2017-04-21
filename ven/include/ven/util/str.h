#pragma once

namespace ven {

  template <size_t size>
  static void copy(cchar_t* s, char_t(&d)[size])
  {
    strncpy_s(d, s, size - 1);
  }

  template <size_t size = 512>
  static std::string make_str(char_t* format, ...)
  {
    char_t buf[size] = { 0, };

    va_list list;
    va_start(list, format);
    vsnprintf_s(buf, size, size - 1, format, list);
    va_end(list);

    return buf;
  }

  template <size_t size>
  static void make_str(char_t(&buf)[size], char_t* format, ...)
  {
    va_list list;
    va_start(list, format);
    vsnprintf_s(buf, size, size - 1, format, list);
    va_end(list);
  }

}
