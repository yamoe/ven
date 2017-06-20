#pragma once

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
    uint16_t month_ = 0;
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
      month_ = st.wMonth;
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
      month_ = st.wMonth;
      day_ = st.wDay;
      hour_ = st.wHour;
      min_ = st.wMinute;
      sec_ = st.wSecond;
      msec_ = st.wMilliseconds;
    }

  };

}
