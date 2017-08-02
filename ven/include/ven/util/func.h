#pragma once


#if defined(WINDOWS)
#else
# include <cstdarg>
# include <cstring>
# include <unistd.h>
# include <sys/stat.h>
# include <iconv.h>
# include <termios.h>
# include <unistd.h>
#endif


// thread
namespace ven {

  static uint64_t tid()
  {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return std::stoull(ss.str());
  }

  static uint64_t tid(std::thread& th)
  {
    std::stringstream ss;
    ss << th.get_id();
    return std::stoull(ss.str());
  }

  static void sleep(int64_t msec)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(msec));
  }

}

// string
namespace ven {

  template <std::size_t size>
  static void copy(const char* s, char(&d)[size])
  {
    strncpy_s(d, s, size - 1);
  }

  template <std::size_t size>
  static void copy(const wchar_t* s, wchar_t(&d)[size])
  {
#if defined(WINDOWS)
    wcsncpy_s(d, s, size - 1);
#else
    wcsncpy(d, s, size - 1);
#endif
  }

  static void copy(const wchar_t* s, wchar_t* d, int size)
  {
#if defined(WINDOWS)
    wcsncpy_s(d, size, s, size - 1);
#else
    wcsncpy(d, s, size - 1);
#endif
  }

  template <std::size_t size = 1024>
  static std::string make_str(const char* format, ...)
  {
    char buf[size] = { 0, };

    va_list list;
    va_start(list, format);
#if defined(WINDOWS)
    vsnprintf_s(buf, size, size - 1, format, list);
#else
    vsnprintf(buf, size, format, list);
#endif
    va_end(list);

    return buf;
  }

  template <std::size_t size>
  static void make_str(char(&buf)[size], const char* format, ...)
  {
    va_list list;
    va_start(list, format);
#if defined(WINDOWS)
    vsnprintf_s(buf, size, size - 1, format, list);
#else
    vsnprintf(buf, size, format, list);
#endif
    va_end(list);
  }

  template <std::size_t size = 1024>
  static std::wstring make_str(wchar_t* format, ...)
  {
    wchar_t buf[size] = { 0, };

    va_list list;
    va_start(list, format);
    _vsnwprintf_s(buf, size, size - 1, format, list);
    va_end(list);

    return buf;
  }

}

// encode character
namespace ven {

#if defined(WINDOWS)
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

#else

  static std::size_t iconv_to(const char* to, const char* from, char* in, std::size_t in_bytes_left, char* out, std::size_t out_bytes_left)
  {
    iconv_t cd = iconv_open(to, from);
    iconv(cd, &in, &in_bytes_left, &out, &out_bytes_left);
    iconv_close(cd);
    return out_bytes_left;
  }

  static std::wstring m2w(const std::string& in)
  {
    std::wstring out(in.size(), 0x00);
    iconv_to(
      "WCHAR_T", "",
      const_cast<char*>(in.c_str()),
      in.size(),
      reinterpret_cast<char*>(const_cast<wchar_t*>(out.c_str())),
      (out.size() * sizeof(wchar_t))
    );
    return out;
  }

  static std::string w2m(const std::wstring& in)
  {
    std::string out(in.size(), 0x00);
    iconv_to(
      "", "WCHAR_T",
      reinterpret_cast<char*>(const_cast<wchar_t*>(in.c_str())),
      (in.size() * sizeof(wchar_t)),
      const_cast<char*>(out.c_str()),
      out.size()
    );
    return out;
  }

  static std::string w2u(const std::wstring& in)
  {
    std::string out(in.size() * 2, 0x00);
    std::size_t bytes_left = iconv_to(
      "UTF-8", "WCHAR_T",
      reinterpret_cast<char*>(const_cast<wchar_t*>(in.c_str())),
      (in.size() * sizeof(wchar_t)),
      const_cast<char*>(out.c_str()),
      out.size()
    );
    out.resize(out.size() - bytes_left);
    return out;
  }

  static std::wstring u2w(const std::string& in)
  {
    std::wstring out(in.size(), 0x00);
    std::size_t bytes_left = iconv_to(
      "WCHAR_T", "UTF-8",
      const_cast<char*>(in.c_str()),
      in.size(),
      reinterpret_cast<char*>(const_cast<wchar_t*>(out.c_str())),
      (out.size() * sizeof(wchar_t))
    );
    out.resize(out.size() - (bytes_left / sizeof(wchar_t)));
    return out;
  }

  static std::string m2u(const std::string& in)
  {
    std::string out(in.size() * 2, 0x00);
    std::size_t bytes_left = iconv_to(
      "UTF-8", "",
      const_cast<char*>(in.c_str()),
      in.size(),
      const_cast<char*>(out.c_str()),
      out.size()
    );
    out.resize(out.size() - bytes_left);
    return out;
  }

  static std::string u2m(const std::string& in)
  {
    std::string out(in.size(), 0x00);
    std::size_t bytes_left = iconv_to(
      "", "UTF-8",
      const_cast<char*>(in.c_str()),
      in.size(),
      const_cast<char*>(out.c_str()),
      out.size()
    );
    out.resize(out.size() - bytes_left);
    return out;
  }

#endif

}

// file
namespace ven {

  static const char* filename(const char* path)
  {
    if (!path) return path;

    const char* p = strrchr(path, '\\');
    if (!p) p = strrchr(path, '/');
    if (p) return p + 1;
    return path;
  }

  static std::string basedir(const char* path)
  {
    if (!path) return path;

    const char* p = strrchr(path, L'\\');
    if (!p) p = strrchr(path, '/');
    if (p) return std::string(path, p);
    return path;
  }

  static const wchar_t* filename(const wchar_t* path)
  {
    if (!path) return path;

    const wchar_t* p = wcsrchr(path, L'\\');
    if (!p) p = wcsrchr(path, L'/');
    if (p) return p + 1;
    return path;
  }


  static std::wstring basedir(const wchar_t* path)
  {
    if (!path) return path;

    const wchar_t* p = wcsrchr(path, L'\\');
    if (!p) p = wcsrchr(path, L'/');
    if (p) return std::wstring(path, p);
    return path;
  }

  static std::string binary_path()
  {
#if defined(WINDOWS)
    char str[256] = { 0, };
    GetModuleFileNameA(NULL, str, 256);
    return str;
#else
    char str[256] = { 0, };
    readlink("/proc/self/exe", str, 256);
    return str;
#endif
  }

  static std::wstring binary_wpath()
  {
#if defined(WINDOWS)
    wchar_t str[256] = { 0, };
    GetModuleFileNameW(NULL, str, 256);
    return str;
#else
    return m2w(binary_path());
#endif
  }

  static std::string binary_filename()
  {
    return filename(binary_path().c_str());
  }

  static std::wstring binary_wfilename()
  {
    return filename(binary_wpath().c_str());
  }

  static bool mkdir(const std::string& path)
  {
    char buf[1024] = { 0, };

    std::size_t len = path.size();
    const char* p = path.c_str();

    for (std::size_t i = 0; i < len; i++) {
      buf[i] = *(p + i);
      if (buf[i] == '\\' || buf[i] == '/' || (i == (len - 1))) {
#if defined(WINDOWS)
        ::CreateDirectoryA(buf, NULL);
#else
        ::mkdir(buf, 0755);
#endif
      }
    }
    return true;
  }

  static bool mkdir(const std::wstring& path)
  {
#if defined(WINDOWS)
    wchar_t buf[1024] = { 0, };

    std::size_t len = path.size();
    const wchar_t* p = path.c_str();

    for (std::size_t i = 0; i < len; i++) {
      buf[i] = *(p + i);
      if (buf[i] == L'\\' || buf[i] == L'/' || (i == (len - 1))) {
        ::CreateDirectoryW(buf, NULL);
      }
    }
#else
    return mkdir(w2u(path));
#endif
    return true;
  }

}

// etc
namespace ven {

  static bool has_console()
  {
#if defined(WINDOWS)
    return (GetStdHandle(STD_INPUT_HANDLE) != NULL);
#else
    return isatty(STDOUT_FILENO);
#endif
  }

#if !defined(WINDOWS)
  static int linux_kbhit(void)
  {
    struct termios oldt, newt;
    int ch;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
  }
#endif

}

