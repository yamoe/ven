#pragma once

namespace ven {
  namespace log {

    enum class LogLevel
    {
      Off,
      Debug,
      Info,
      Warn,
      Error,
    };


    static char* str(LogLevel level)
    {
      switch (level)
      {
      case LogLevel::Off: return "O";
      case LogLevel::Debug: return "D";
      case LogLevel::Info: return "I";
      case LogLevel::Warn: return "W";
      case LogLevel::Error: return "E";
      default: return "U";  // unknown
      }
    }


    class LogInfo
    {
    public:
      int32_t tid_ = 0;
      const char* func_;
      const char* file_;
      const char* filename_;
      int32_t line_;
      Time time_;
      LogLevel level_;
      std::string msg_;
    };



    class LogFile
    {
    private:
      FILE* file_ = nullptr;
      std::wstring path_;

      uint64_t size_ = 0;

    public:
      LogFile() {}

      ~LogFile()
      {
        close();
      }

      uint64_t size()
      {
        return size_;
      }

      void open(const std::wstring& path)
      {
        close();
        path_ = path;
        file_ = _wfsopen(path.c_str(), L"ab", _SH_DENYWR);
        if (file_) {
          fseek(file_, 0L, SEEK_END);
          size_ = std::ftell(file_);
        }
      }

      void close()
      {
        if (valid()) {
          flush();
          size_ = 0;
          std::fclose(file_);
          file_ = nullptr;
        }
      }

      template <int size>
      void write(CharArray<size>& carr)
      {
        if (!valid()) return;

        size_t writed = std::fwrite(
          carr.ch(),
          1,
          static_cast<size_t>(carr.len()),
          file_
        );
        size_ += writed;
      }

      void flush()
      {
        if (valid()) {
          std::fflush(file_);
        }
      }

      operator bool() { return valid(); }

      bool valid() { return (file_ != nullptr); }
    };


    class LogEvent
    {
    public:
      const bool has_console_ = has_console();

    private:
      bool use_console_ = true;
      bool async_ = false;
      LogLevel level_ = LogLevel::Debug;

    public:
      LogEvent(
        LogLevel level = LogLevel::Debug,
        bool use_console = true,
        bool async = false
      )
        : level_(level)
        , use_console_(use_console)
        , async_(async)
      {}

      bool use_console() { return use_console_; }
      bool async() { return async_; }
      LogLevel level() { return level_; }

      virtual void write(LogFile& file, LogInfo& line) = 0;

      void write_default(LogFile& file, LogInfo& info)
      {
        CharArray<1024> arr;
        arr.add(
          "[%04d-%02d-%02d %02d:%02d:%02d.%03d %p %s] %s (%s:%d)\r\n",
          info.time_.year_, info.time_.month_, info.time_.day_,
          info.time_.hour_, info.time_.min_, info.time_.sec_, info.time_.msec_,
          info.tid_, str(info.level_),
          info.msg_.c_str(),
          info.filename_, info.line_
        );

        if (has_console_ && use_console_) {
          printf("%s", arr.ch());
        }

        file.write(arr);
        //file.flush();
      }
    };


    class ILogger
    {
    public:
      virtual void log(const char* func, const char* file, int32_t line, LogLevel level, char* msg) = 0;
      virtual void write(LogInfo& info) = 0;
    };

    class Log
    {
    public:
      LogLevel level_ = LogLevel::Off;
      ILogger* logger_ = nullptr;
      const char* func_ = nullptr;
      const char* file_ = nullptr;
      int32_t line_ = 0;

      Log(
        ILogger* logger,
        LogLevel level = LogLevel::Off,
        const char* func = nullptr,
        const char* file = nullptr,
        int32_t line = 0
      )
        : logger_(logger)
        , level_(level)
        , func_(func)
        , file_(file)
        , line_(line)
      {}

      template <size_t size = 1024>
      void operator()(char* format, ...)
      {
        if (!logger_ || level_ == LogLevel::Off) {
          return;
        }

        char buf[size] = { 0, };
        va_list list;
        va_start(list, format);
        vsnprintf_s(buf, size, size - 1, format, list);
        va_end(list);

        logger_->log(func_, file_, line_, level_, buf);
      }
    };


    class LogThread : public Thread
    {
    private:
      ILogger& logger_;
      SLock lock_;
      std::deque<LogInfo> q1_;
      std::deque<LogInfo> q2_;

    public:

      LogThread(ILogger& logger)
        : logger_(logger)
      {
      }

      void add(LogInfo& info)
      {
        VEN_LOCKER(lock_);
        q1_.push_back(info);
      }

    protected:
      virtual void run() override
      {
        while (true) {
          loop();
          Sleep(1);
        }
      }

    private:
      void loop()
      {
        swap();
        if (q2_.empty()) {
          return;
        }

        for (auto& info : q2_)
        {
          logger_.write(info);
        }
        q2_.clear();
      }

      void swap()
      {
        VEN_LOCKER(lock_);
        q1_.swap(q2_);
      }
    };


    class Logger
      : public ILogger
    {
    private:
      friend class Log;

    protected:
      LogLevel level_ = LogLevel::Debug;

      SLock lock_;
      LogFile file_;

      LogEvent* evt_ = nullptr;
      LogThread* th_ = nullptr;

    public:
      Logger() {}

      void init(LogEvent* evt)
      {
        evt_ = evt;

        if (evt->async()) {
          th_ = new LogThread(*this);
          th_->start();
        }
      }

      Log log(LogLevel level, const char* func, const char* file, int32_t line)
      {
        if (level < level_) {
          return Log(nullptr);
        }
        return Log(this, level, func, file, line);
      }

      bool is_async()
      {
        return (th_ != nullptr);
      }

    private:
      virtual void log(const char* func, const char* file, int32_t line, LogLevel level, char* msg)
      {
        LogInfo info;
        info.tid_ = ::GetCurrentThreadId();
        info.func_ = func;
        info.file_ = file;
        info.filename_ = filename(file);
        info.line_ = line;
        info.level_ = level;
        info.time_ = Time::now();
        info.msg_ = msg;

        if (is_async()) {
          th_->add(info);
        } else {
          write(info);
        }
      }

      virtual void write(LogInfo& info)
      {
        VEN_LOCKER(lock_);
        evt_->write(file_, info);
      }

    };


    class DailyLog : public LogEvent
    {
    private:
      Time next_time_;

    public:
      DailyLog(
        int32_t hour,
        int32_t min,
        LogLevel level = LogLevel::Debug,
        bool use_console = true,
        bool async = false
      )
        : LogEvent(level, use_console, async)
      {
        next_time_ = Time::today(hour, min);
        if (next_time_ <= Time::now()) {
          next_time_.add_day();
        }
      }

      std::wstring path(Time& t)
      {
        return make_str(
          L"%s_%04d%02d%02d_%02d%02d.log",
          binary_wpath().c_str(),
          t.year_, t.month_, t.day_, t.hour_, t.min_
        );
      }

      virtual void write(LogFile& file, LogInfo& info) override
      {
        // 처음 호출된 경우
        if (!file) {
          Time t = next_time_;
          file.open(path(t.add_day(-1)));
        }

        // 시간 초과한 경우
        if (info.time_ >= next_time_) {
          file.open(path(next_time_));
          next_time_.add_day();
        }

        write_default(file, info);
      }
    };


    class RollingLog : public LogEvent
    {
    private:
      uint64_t size_ = 0;
      uint32_t cnt_ = 0;

      uint32_t cur_cnt_ = 0;

    public:
      RollingLog(
        uint64_t size, // bytes
        uint32_t cnt,
        LogLevel level = LogLevel::Debug,
        bool use_console = true,
        bool async = false
      )
        : LogEvent(level, use_console, async)
        , size_(size)
        , cnt_(cnt)
      {}

      std::wstring path()
      {
        cur_cnt_++;
        if (cur_cnt_ > cnt_) {
          cur_cnt_ = 1;
        }

        return make_str(
          L"%s_%02d.log",
          binary_wpath().c_str(),
          cur_cnt_
        );
      }

      virtual void write(LogFile& file, LogInfo& info) override
      {
        // 처음 호출된 경우
        if (!file) {
          file.open(path());
        }

        // 용량 초과한 경우
        if (file.size() >= size_) {
          file.open(path());
        }

        write_default(file, info);
      }
    };

  }
}


#define VENL_DECL(name) \
class VENL_##name : public ven::log::Logger \
{ \
  public: \
  static VENL_##name& inst() \
  { \
    static VENL_##name inst_; \
    return inst_; \
  } \
}

#define VENL_INIT(name) \
VENL_##name::inst().init

#define VENL_D(name) \
VENL_##name::inst().log(ven::log::LogLevel::Debug, __FUNCTION__, __FILE__, __LINE__)

#define VENL_I(name) \
VENL_##name::inst().log(ven::log::LogLevel::Info, __FUNCTION__, __FILE__, __LINE__)

#define VENL_W(name) \
VENL_##name::inst().log(ven::log::LogLevel::Warn, __FUNCTION__, __FILE__, __LINE__)

#define VENL_E(name) \
VENL_##name::inst().log(ven::log::LogLevel::Error, __FUNCTION__, __FILE__, __LINE__)

/*
** rolling example

  VENL_DECL(Roll);

  VENL_INIT(Roll)(
    new ven::log::RollingLog(1024 * 10, 3, ven::log::LogLevel::Debug, true, false)
  );
  VENL_D(Roll)("hi");
  VENL_I(Roll)("hi");
  VENL_W(Roll)("hi");
  VENL_E(Roll)("hi");


** daily example

  VENL_DECL(Daily);

  VENL_INIT(Daily)(
    new ven::log::DailyLog(15, 51, ven::log::LogLevel::Debug, true, false)
  );
  VENL_D(Daily)("hi");
  VENL_I(Daily)("hi");
  VENL_W(Daily)("hi");
  VENL_E(Daily)("hi");

*/