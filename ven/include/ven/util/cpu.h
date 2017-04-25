#pragma once

namespace ven {

  static ui32_t cpu_count()
  {
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return static_cast<ui32_t>(info.dwNumberOfProcessors);
  }

}
