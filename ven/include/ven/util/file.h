#pragma once

namespace ven {

  class File
  {
  private:
    HANDLE handle_ = INVALID_HANDLE_VALUE;
  public:
    File() {}

    File(HANDLE handle)
    {
      handle_ = handle;
    }

    File(const std::wstring& path)
    {
      open(path);
    }

    ~File()
    {
      close();
    }

    void open(const std::wstring& path)
    {
      close();
      handle_ = CreateFileW(
        path.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_FLAG_WRITE_THROUGH,
        NULL
      );
    }

    void close()
    {
      if (valid()) {
        CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
      }
    }

    void write(const std::string& content)
    {
      if (!valid()) return;

      DWORD writed = 0;
      WriteFile(
        handle_,
        content.c_str(),
        static_cast<DWORD>(content.size()),
        &writed,
        0
      );
      FlushFileBuffers(handle_);
    }

    template <int size>
    void write(CharArray<size>& carr)
    {
      if (!valid()) return;

      DWORD writed = 0;
      WriteFile(
        handle_,
        carr.ch(),
        static_cast<DWORD>(carr.len()),
        &writed,
        0
      );
      FlushFileBuffers(handle_);
    }

    operator HANDLE() { return handle_; }

    operator bool() { return valid(); }

    bool valid() { return (handle_ != INVALID_HANDLE_VALUE); }

  };

}
