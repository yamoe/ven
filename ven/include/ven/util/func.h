﻿#pragma once

namespace ven {

#pragma region string
  template <size_t size>
  static void copy(cchar_t* s, char_t(&d)[size])
  {
    strncpy_s(d, s, size - 1);
  }

  template <size_t size = 1024>
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

  template <size_t size = 1024>
  static std::wstring make_str(wchar_t* format, ...)
  {
    wchar_t buf[size] = { 0, };

    va_list list;
    va_start(list, format);
    _vsnwprintf_s(buf, size, size - 1, format, list);
    va_end(list);

    return buf;
  }

#pragma endregion

#pragma region file

  static cchar_t* filename(cchar_t* path)
  {
    if (!path) return path;

    cchar_t* p = strrchr(path, '\\');
    if (p) return p + 1;
    return path;
  }

  static std::string basedir(cchar_t* path)
  {
    if (!path) return path;

    cchar_t* p = strrchr(path, L'\\');
    if (p) return std::string(path, p);
    return path;
  }

  static cwchar_t* filename(cwchar_t* path)
  {
    if (!path) return path;

    cwchar_t* p = wcsrchr(path, L'\\');
    if (p) return p + 1;
    return path;
  }


  static std::wstring basedir(cwchar_t* path)
  {
    if (!path) return path;

    cwchar_t* p = wcsrchr(path, L'\\');
    if (p) return std::wstring(path, p);
    return path;
  }

  static std::string binary_path()
  {
    char_t str[MAX_PATH] = { 0, };
    GetModuleFileNameA(NULL, str, MAX_PATH);
    return str;
  }

  static std::wstring binary_wpath()
  {
    wchar_t str[MAX_PATH] = { 0, };
    GetModuleFileNameW(NULL, str, MAX_PATH);
    return str;
  }

  static bool mkdir(const std::wstring& path)
  {
    wchar_t buf[1024] = { 0, };

    size_t len = path.size();
    cwchar_t* p = path.c_str();

    for (size_t i = 0; i < len; i++) {
      buf[i] = *(p + i);
      if (buf[i] == L'\\' || buf[i] == L'/' || (i == (len - 1)) ) {
        ::CreateDirectoryW(buf, NULL);
      }
    }
    return true;
  }

#pragma endregion

#pragma region encode character

  // multibyte -> wide
  static std::wstring m2w(const std::string& in)
  {
    int len = ::MultiByteToWideChar(CP_ACP, 0, in.c_str(), -1, 0, 0);
    std::wstring out(len, 0);
    ::MultiByteToWideChar(CP_ACP, 0, in.c_str(), -1, const_cast<wchar_t*>(out.c_str()), len);
    return out;
  }

  // wide -> multibyte
  static std::string w2m(const std::wstring& in)
  {
    int len = ::WideCharToMultiByte(CP_ACP, 0, in.c_str(), -1, 0, 0, 0, 0);
    std::string out(len, 0);
    ::WideCharToMultiByte(CP_ACP, 0, in.c_str(), -1, const_cast<char*>(out.c_str()), len, 0, 0);
    return out;
  }

  // wide -> utf8
  static std::string w2u(const std::wstring& in)
  {
    int len = ::WideCharToMultiByte(CP_UTF8, 0, in.c_str(), -1, 0, 0, 0, 0);
    std::string out(len, 0);
    ::WideCharToMultiByte(CP_UTF8, 0, in.c_str(), -1, const_cast<char*>(out.c_str()), len, 0, 0);
    return out;
  }

  // utf8 -> wide
  static std::wstring u2w(const std::string& in)
  {
    int len = ::MultiByteToWideChar(CP_UTF8, 0, in.c_str(), -1, 0, 0);
    std::wstring out(len, 0);
    ::MultiByteToWideChar(CP_UTF8, 0, in.c_str(), -1, const_cast<wchar_t*>(out.c_str()), len);
    return out;
  }

  // multibye -> utf8
  static std::string m2u(const std::string& in)
  {
    return w2u(m2w(in));
  }

  // utf8 -> multibye
  static std::string u2m(const std::string& in)
  {
    return w2m(u2w(in));
  }

#pragma endregion

}