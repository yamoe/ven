#pragma once

namespace ven {

  class NonCopyable {
  public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    void operator=(const NonCopyable& c) = delete;
  };

}