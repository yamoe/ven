#pragma once

namespace ven
{

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
      handle_ = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    void close()
    {
      if (vaild()) {
        CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
      }
    }

    bool write(const std::string& content)
    {
      if (!vaild()) return false;

      DWORD writed = 0;
      return (WriteFile(handle_, content.c_str(), static_cast<DWORD>(content.size()), &writed, 0) == TRUE);
    }

    operator HANDLE() { return handle_; }

    operator bool()
    {
      return vaild();
    }

    bool vaild()
    {
      return (handle_ != INVALID_HANDLE_VALUE);
    }

  };

}
