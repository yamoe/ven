#pragma once

namespace ven {

  static uint32_t cpu_count()
  {
#if defined(WINDOWS)
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return static_cast<uint32_t>(info.dwNumberOfProcessors);
#else
    return std::thread::hardware_concurrency();
#endif
  }

}
