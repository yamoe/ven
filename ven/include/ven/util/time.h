#pragma once

#include <time.h>

#if defined(WINDOWS)
#else
# include <sys/time.h>
#endif

#if defined(WINDOWS)
namespace ven {

  class Time
  {
  public:
    static const uint32_t Sec = 1000;
  public:
    uint32_t year_ = 0;
    uint32_t mon_ = 0;
    uint32_t day_ = 0;
    uint32_t hour_ = 0;
    uint32_t min_ = 0;
    uint32_t sec_ = 0;
    uint32_t msec_ = 0;

  public:
    Time() {}

    ~Time() {}

    bool operator==(const Time& rhs) { return !(*this != rhs); }
    bool operator!=(const Time& rhs)
    {
      return
        (msec_ != rhs.msec_) || (sec_ != rhs.sec_) || (min_ != rhs.min_) ||
        (hour_ != rhs.hour_) || (day_ != rhs.day_) || (mon_ != rhs.mon_) ||
        (year_ != rhs.year_);
    }
    bool operator<=(const Time& rhs) { return (*this == rhs) || (*this < rhs); }
    bool operator>=(const Time& rhs) { return (*this == rhs) || (*this > rhs); }
    bool operator>(const Time& rhs) { return !(*this < rhs) && !(*this == rhs); }
    bool operator<(const Time& rhs)
    {
      return
        (year_ < rhs.year_) ? true : (year_ > rhs.year_) ? false :
        (mon_ < rhs.mon_) ? true : (mon_ > rhs.mon_) ? false :
        (day_ < rhs.day_) ? true : (day_ > rhs.day_) ? false :
        (hour_ < rhs.hour_) ? true : (hour_ > rhs.hour_) ? false :
        (min_ < rhs.min_) ? true : (min_ > rhs.min_) ? false :
        (sec_ < rhs.sec_) ? true : (sec_ > rhs.sec_) ? false :
        (msec_ < rhs.msec_) ? true : (msec_ > rhs.msec_) ? false :
        (msec_ == rhs.msec_) ? false : false;
    }

    Time& add(int32_t day, int32_t hour, int32_t min, int32_t sec, int32_t msec)
    {
      struct tm tm;
      tm.tm_year = year_ - 1900;
      tm.tm_mon = mon_ - 1;
      tm.tm_mday = day_;
      tm.tm_hour = hour_;
      tm.tm_min = min_;
      tm.tm_sec = sec_;
      time_t tt = mktime(&tm);

      msec_ += msec;
      if (msec_ >= Sec) {
        tt += msec_ / Sec;
        msec_ = msec_ % Sec;
      }

      if (day != 0) tt += day * 60 * 60 * 24;
      if (hour != 0) tt += hour * 60 * 60;
      if (min != 0) tt += min * 60;
      if (sec != 0) tt += sec;

      tm = localtime_tm(static_cast<int64_t>(tt));

      year_ = tm.tm_year + 1900;
      mon_ = tm.tm_mon + 1;
      day_ = tm.tm_mday;
      hour_ = tm.tm_hour;
      min_ = tm.tm_min;
      sec_ = tm.tm_sec;

      return *this;
    }

    Time& add_day(int32_t day = 1)
    {
      add(day, 0, 0, 0, 0);
      return *this;
    }

  public:
    static Time now()
    {
#if defined(WINDOWS)
      SYSTEMTIME st;
      GetLocalTime(&st);

      Time t;
      t.year_ = st.wYear;
      t.mon_ = st.wMonth;
      t.day_ = st.wDay;
      t.hour_ = st.wHour;
      t.min_ = st.wMinute;
      t.sec_ = st.wSecond;
      t.msec_ = st.wMilliseconds;
      return t;

#else
      struct timeval val;
      gettimeofday(&val, nullptr);
      struct tm* ptm = localtime(&val.tv_sec);

      Time t;
      t.year_ = ptm->tm_year + 1900;
      t.mon_ = ptm->tm_mon + 1;
      t.day_ = ptm->tm_mday;
      t.hour_ = ptm->tm_hour;
      t.min_ = ptm->tm_min;
      t.sec_ = ptm->tm_sec;
      t.msec_ = val.tv_usec / 1000;
      return t;
#endif
    }

    static tm localtime_tm(int64_t tt /* time_t */)
    {
      struct tm tm;
#if defined(WINDOWS)
      localtime_s(&tm, &tt);
#else
      struct tm* ptm = localtime(&tt);
      tm = *ptm;
#endif
      return tm;
    }

    static Time today(int32_t hour, int32_t min)
    {
      Time t = now();
      t.hour_ = hour;
      t.min_ = min;
      t.sec_ = 0;
      t.msec_ = 0;
      return t;
    }

  };

}

#else

namespace ven {

  class Time
  {
    static const uint64_t Msec = 10000;
    static const uint64_t Sec = Msec * 1000;
    static const uint64_t Min = Sec * 60;
    static const uint64_t Hour = Min * 60;
    static const uint64_t Day = Hour * 24;
    static const uint64_t Year = Day * 365;

  public:
    uint64_t time_ = 0;

    uint16_t year_ = 0;
    uint16_t mon_ = 0;
    uint16_t day_ = 0;
    uint16_t hour_ = 0;
    uint16_t min_ = 0;
    uint16_t sec_ = 0;
    uint16_t msec_ = 0;

  public:
    Time() {}

    Time(SYSTEMTIME& st)
    {
      set(st);
    }
    
    bool operator<(const Time& rhs) { return time_ < rhs.time_; }
    bool operator>(const Time& rhs) { return time_ > rhs.time_; }
    bool operator<=(const Time& rhs) { return time_ <= rhs.time_; }
    bool operator>=(const Time& rhs) { return time_ >= rhs.time_; }
    bool operator==(const Time& rhs) { return time_ == rhs.time_; }
    bool operator!=(const Time& rhs) { return time_ != rhs.time_; }

    void add(int32_t hour, int32_t min, int32_t sec, int32_t msec)
    {
      uint64_t t =
        time_ +
        (static_cast<uint64_t>(hour) * Hour) +
        (static_cast<uint64_t>(min) * Min) +
        (static_cast<uint64_t>(sec) * Sec) +
        (static_cast<uint64_t>(msec) * Msec);
      set(t);
    }

    Time& add_day(int32_t day = 1)
    {
      uint64_t t =
        time_ +
        ((static_cast<uint64_t>(day) * 24) * Hour);
      set(t);
      return *this;
    }


  public:
    static Time now()
    {
      SYSTEMTIME st;
      GetLocalTime(&st);
      return st;
    }

    static Time today(int32_t hour, int32_t min)
    {
      SYSTEMTIME st;
      GetLocalTime(&st);
      st.wHour = hour;
      st.wMinute = min;
      st.wSecond = 0;
      st.wMilliseconds = 0;
      
      Time t;
      t.set(st);
      return t;
    }

  private:
    void set(SYSTEMTIME& st)
    {
      FILETIME ft;
      SystemTimeToFileTime(&st, &ft);

      ULARGE_INTEGER i;
      i.HighPart = ft.dwHighDateTime;
      i.LowPart = ft.dwLowDateTime;

      time_ = i.QuadPart;
      year_ = st.wYear;
      mon_ = st.wMonth;
      day_ = st.wDay;
      hour_ = st.wHour;
      min_ = st.wMinute;
      sec_ = st.wSecond;
      msec_ = st.wMilliseconds;
    }

    void set(uint64_t t)
    {
      ULARGE_INTEGER i;
      i.QuadPart = t;

      FILETIME ft;
      ft.dwHighDateTime = i.HighPart;
      ft.dwLowDateTime = i.LowPart;

      SYSTEMTIME st;
      FileTimeToSystemTime(&ft, &st);

      time_ = t;
      year_ = st.wYear;
      mon_ = st.wMonth;
      day_ = st.wDay;
      hour_ = st.wHour;
      min_ = st.wMinute;
      sec_ = st.wSecond;
      msec_ = st.wMilliseconds;
    }

  };

}
#endif

