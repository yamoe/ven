#pragma once

#if defined(WINDOWS)
#else
# include <pthread>
#endif

namespace ven {

  class SLock {
  private:
#if defined(WINDOWS)
    CRITICAL_SECTION lock_;
#else
    pthread_spinlock_t lock_;
#endif

  public:
    SLock(unsigned int spin_count = 4000)
    {
#if defined(WINDOWS)
      InitializeCriticalSectionAndSpinCount(&lock_, spin_count);
#else
      pthread_spin_init(&lock_, 0);
#endif
    }

    ~SLock()
    {
#if defined(WINDOWS)
      DeleteCriticalSection(&lock_);
#else
      pthread_spin_destroy(&lock_);
#endif
    }

    void lock()
    {
#if defined(WINDOWS)
      EnterCriticalSection(&lock_);
#else
      pthread_spin_lock(&lock_);
#endif
    }

    void unlock()
    {
#if defined(WINDOWS)
      LeaveCriticalSection(&lock_);
#else
      pthread_spin_unlock(&lock_);
#endif
    }
  };


  class SLocker {
  private:
    SLock& lock_;

  public:
    SLocker(SLock& lock)
      : lock_(lock)
    {
      lock_.lock();
    }

    ~SLocker()
    {
      lock_.unlock();
    }
  };



#define _VEN_STR(s) #s
#define VEN_STR(s) _VEN_STR(s)

#define _VEN_CONCAT(a, b) a ## b
#define VEN_CONCAT(a, b) _VEN_CONCAT(a, b)

#define VEN_SLOCKER_VAR_NAME \
  VEN_CONCAT(\
    VEN_CONCAT(\
      VEN_CONCAT(locker_, __COUNTER__)\
      , _\
     )\
  , __LINE__)

#define VEN_BRACE_TEMP_VAR_NAME VEN_CONCAT(locker_temp_bool_, __LINE__)

#define VEN_LOCKER(lock) ven::SLocker VEN_SLOCKER_VAR_NAME(lock)

#define VEN_BRACE_LOCKER(lock)\
  bool VEN_BRACE_TEMP_VAR_NAME = true;\
  for (VEN_LOCKER(lock); VEN_BRACE_TEMP_VAR_NAME == true; VEN_BRACE_TEMP_VAR_NAME = false)


}
