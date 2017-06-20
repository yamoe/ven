#pragma once

namespace ven {

  static uint32_t cpu_count()
  {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return static_cast<uint32_t>(info.dwNumberOfProcessors);
  }

}
