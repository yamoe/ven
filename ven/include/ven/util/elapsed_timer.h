#pragma once

namespace ven {

  class ElapsedTimer {
  private:
#if defined(WINDOWS)
    LARGE_INTEGER freq_ = { 0 };
    LARGE_INTEGER start_ = { 0 };
#else
    typedef std::chrono::high_resolution_clock Clock;
    typedef Clock::time_point Time;
    Time start_;
#endif

  public:
    ElapsedTimer()
    {
#if defined(WINDOWS)
      QueryPerformanceFrequency(&freq_);
#endif
    }

    void start()
    {
#if defined(WINDOWS)
      QueryPerformanceCounter(&start_);
#else
      start_ = Clock::now();
#endif
    }

    double stop()
    {
      //mesc
#if defined(WINDOWS)
      LARGE_INTEGER stop;
      QueryPerformanceCounter(&stop);
      return ((double)(stop.QuadPart - start_.QuadPart) / freq_.QuadPart) * 1000;
#else
      double t = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - start_).count());
      return t / 1000.0;
#endif
    }

  };

}

