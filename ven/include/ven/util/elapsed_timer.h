#pragma once

namespace ven {

  class ElapsedTimer {
  private:
    LARGE_INTEGER freq_ = { 0 };
    LARGE_INTEGER start_ = { 0 };

  public:
    ElapsedTimer()
    {
      QueryPerformanceFrequency(&freq_);
    }

    void start()
    {
      QueryPerformanceCounter(&start_);
    }

    double stop()
    {
      LARGE_INTEGER stop;
      QueryPerformanceCounter(&stop);

      //mesc
      return ((double)(stop.QuadPart - start_.QuadPart) / freq_.QuadPart) * 1000;
    }

  };

}

