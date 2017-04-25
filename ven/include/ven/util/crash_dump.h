#pragma once

#include <signal.h>
#include <new.h>

namespace ven
{

  class CrashDump
  {
  private:
    static const DWORD EXCEPTION_THROW_EXCEPTION = 0xE06D7363;

    
    static const DWORD EXCEPTION_INVALID_PARAMETER = 0xFFFFFFF1;
    static const DWORD EXCEPTION_PURE_CALL = 0xFFFFFFF2;
    static const DWORD EXCEPTION_OUT_OF_MEMORY = 0xFFFFFFF3;
    static const DWORD EXCEPTION_TERMINATE = 0xFFFFFFF4;
    static const DWORD EXCEPTION_UNEXPECTED = 0xFFFFFFF5;

    static const DWORD EXCEPTION_SIGABRT = 0xFFFFFFF6;
    static const DWORD EXCEPTION_SIGINT = 0xFFFFFFF7;
    static const DWORD EXCEPTION_SIGTERM = 0xFFFFFFF8;
    static const DWORD EXCEPTION_SIGFPE = 0xFFFFFFF9;
    static const DWORD EXCEPTION_SIGILL = 0xFFFFFF10;
    static const DWORD EXCEPTION_SIGSEGV = 0xFFFFFF11;

    static const DWORD EXCEPTION_USER_CREATED_NORMAL = 0xFFFFFF12;
    static const DWORD EXCEPTION_USER_CREATED_FULL_DUMP = 0xFFFFFF13;

    class Conf
    {
    public:
      std::wstring base_path_;
      int type_ = MiniDumpNormal; // MINIDUMP_TYPE

      void set_base_path(const std::wstring& path)
      {
        if (path.empty()) {
          base_path_ = binary_wpath();

        } else {
          mkdir(path);

          base_path_ = make_str(
            L"%s\\%s",
            path.c_str(),
            filename(binary_wpath().c_str())
          );

        }
      }

      void set_type(bool full_dump)
      {
        if (full_dump) {
          type_ = MiniDumpWithFullMemory | MiniDumpWithFullMemoryInfo | MiniDumpWithThreadInfo;
        } else {
          type_ = MiniDumpNormal;
        }
      }

    };

    class RunParam
    {
    public:
      EXCEPTION_POINTERS* ep_;
      Conf& conf_;
    public:
      RunParam(EXCEPTION_POINTERS* ep, Conf& conf)
        : ep_(ep), conf_(conf)
      {}
    };

  public:
    CrashDump() {}
    ~CrashDump() {}

    static void install(const std::wstring& base_dir = L"", bool full_dump = false)
    {
      Conf& c = conf();
      c.set_base_path(base_dir);
      c.set_type(full_dump);

      // ** process
      SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)seh_handler);
      //if (PreventSetUnhandledExceptionFilter() == FALSE) {
      //  printf("fail set prevent exception filter\n");
      //}

      _set_invalid_parameter_handler(CrashDump::invalid_parameter_handler);
      _set_purecall_handler(CrashDump::purecall_handler);
      _set_new_handler(new_handler);
      _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
      signal(SIGABRT, sigabrt_handler); // abort();
      signal(SIGINT, sigint_handler); // raise(SIGINT);
      signal(SIGTERM, sigterm_handler); // raise(SIGTERM)

      install_thread();
    }

    static void install_thread()
    {
      set_terminate(terminate_handler); // terminate();
      set_unexpected(unexpected_handler); //unexpected();
      signal(SIGFPE, sigfpe_handler); // floating point exception
      signal(SIGILL, sigill_handler); // raise(SIGILL);
      signal(SIGSEGV, sigsegv_handler); //  raise(SIGSEGV);
    }

    static void dump()
    {
      Conf c = conf();
      c.set_type(false);

      create_dump(MakeEP(EXCEPTION_USER_CREATED_NORMAL), c);
    }

    static void full_dump()
    {
      Conf c = conf();
      c.set_type(true);

      create_dump(MakeEP(EXCEPTION_USER_CREATED_FULL_DUMP), c);
    }

  private:

    static Conf& conf()
    {
      static Conf conf;
      return conf;
    }

    static void handler(DWORD code)
    {
      create_dump(MakeEP(code));
      TerminateProcess(GetCurrentProcess(), 1);
    }

    static void invalid_parameter_handler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t reserved)
    {
      handler(EXCEPTION_INVALID_PARAMETER);
    }

    static void purecall_handler()
    {
      handler(EXCEPTION_PURE_CALL);
    }

    static int new_handler(size_t size)
    {
      // TODO size 를 남기면 좋겠는데?
      handler(EXCEPTION_OUT_OF_MEMORY);
      return 0;
    }

    static void sigabrt_handler(int)
    {
      handler(EXCEPTION_SIGABRT);
    }

    static void sigterm_handler(int)
    {
      handler(EXCEPTION_SIGINT);
    }

    static void sigint_handler(int)
    {
      handler(EXCEPTION_SIGTERM);
    }

    static void terminate_handler()
    {
      handler(EXCEPTION_TERMINATE);
    }

    static void unexpected_handler()
    {
      handler(EXCEPTION_UNEXPECTED);
    }

    static void sigfpe_handler(int)
    {
      //(EXCEPTION_POINTERS*)_pxcptinfoptrs;
      handler(EXCEPTION_SIGFPE);
    }

    static void sigill_handler(int)
    {
      handler(EXCEPTION_SIGILL);
    }

    static void sigsegv_handler(int)
    {
      //(EXCEPTION_POINTERS*)_pxcptinfoptrs;
      handler(EXCEPTION_SIGSEGV);
    }

    static LONG seh_handler(EXCEPTION_POINTERS* ep)
    {
      if (!ep) return EXCEPTION_EXECUTE_HANDLER;

      create_dump(ep);
      return EXCEPTION_EXECUTE_HANDLER;
    }

    static void create_dump(EXCEPTION_POINTERS* ep, Conf& conf = conf())
    {
      RunParam param(ep, conf);

      if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW) {

        HANDLE handle = reinterpret_cast<HANDLE>(
          _beginthreadex(0, 0, (unsigned(__stdcall*)(void*))run, &param, 0, 0)
        );
        WaitForSingleObject(handle, INFINITE);
        CloseHandle(handle);
      } else {
        run(&param);
      }
    }

    static uintptr_t __stdcall run(void* arg)
    {
      RunParam& param = *reinterpret_cast<RunParam*>(arg);

      static std::atomic<ui32_t> cnt_ = 0;
      ui32_t cnt = ++cnt_;

      SYSTEMTIME now = { 0, };
      GetLocalTime(&now);

      std::wstring path = make_str(
        L"%s_%04d%02d%02d_%02d%02d%02d.%03d_%d",
        param.conf_.base_path_.c_str(),
        now.wYear, now.wMonth, now.wDay,
        now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
        cnt
      );

      File file(path + L".txt");
      if (!file) {
        printf("invalid info filel : %d\n", GetLastError());
      }

      // write time
      file.write(
        make_str(
          "\n%04d-%02d-%02d %02d:%02d:%02d.%03d (%d)\n",
          now.wYear, now.wMonth, now.wDay,
          now.wHour, now.wMinute, now.wSecond, now.wMilliseconds,
          cnt
        )
      );

      // write exception code
      file.write(
        make_str(
          "\n%s(0x%08X)\n\n",
          ecode(param.ep_->ExceptionRecord->ExceptionCode),
          param.ep_->ExceptionRecord->ExceptionCode
        )
      );

      // write stack trace
      file.write(
        StackTrace().trace(param.ep_->ContextRecord)
      );

      // write minidump file
      File dump_file(path + L".dmp");
      if (!dump_file) {
        file.write(
          make_str("\n\nfail open dump file : %d\n\n", GetLastError())
        );
      } else {

        MINIDUMP_EXCEPTION_INFORMATION info;
        info.ThreadId = GetCurrentThreadId();
        info.ExceptionPointers = param.ep_;
        info.ClientPointers = FALSE;

        if (MiniDumpWriteDump(
          GetCurrentProcess(),
          GetCurrentProcessId(),
          dump_file,
          static_cast<MINIDUMP_TYPE>(param.conf_.type_),
          &info,
          NULL,
          NULL
        ) == FALSE) {
          file.write(
            make_str("\n\nfail write dump file : %d\n\n", GetLastError())
          );
        }
      }

      return 0;
    }

    static char* ecode(DWORD code)
    {
      switch (code) {
      case EXCEPTION_ACCESS_VIOLATION: return "EXCEPTION_ACCESS_VIOLATION";
      case EXCEPTION_DATATYPE_MISALIGNMENT: return "EXCEPTION_DATATYPE_MISALIGNMENT";
      case EXCEPTION_BREAKPOINT: return "EXCEPTION_BREAKPOINT";
      case EXCEPTION_SINGLE_STEP: return "EXCEPTION_SINGLE_STEP";
      case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
      case EXCEPTION_FLT_DENORMAL_OPERAND: return "EXCEPTION_FLT_DENORMAL_OPERAND";
      case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
      case EXCEPTION_FLT_INEXACT_RESULT: return "EXCEPTION_FLT_INEXACT_RESULT";
      case EXCEPTION_FLT_INVALID_OPERATION: return "EXCEPTION_FLT_INVALID_OPERATION";
      case EXCEPTION_FLT_OVERFLOW: return "EXCEPTION_FLT_OVERFLOW";
      case EXCEPTION_FLT_STACK_CHECK: return "EXCEPTION_FLT_STACK_CHECK";
      case EXCEPTION_FLT_UNDERFLOW: return "EXCEPTION_FLT_UNDERFLOW";
      case EXCEPTION_INT_DIVIDE_BY_ZERO: return "EXCEPTION_INT_DIVIDE_BY_ZERO";
      case EXCEPTION_INT_OVERFLOW: return "EXCEPTION_INT_OVERFLOW";
      case EXCEPTION_PRIV_INSTRUCTION: return "EXCEPTION_PRIV_INSTRUCTION";
      case EXCEPTION_IN_PAGE_ERROR: return "EXCEPTION_IN_PAGE_ERROR";
      case EXCEPTION_ILLEGAL_INSTRUCTION: return "EXCEPTION_ILLEGAL_INSTRUCTION";
      case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
      case EXCEPTION_STACK_OVERFLOW: return "EXCEPTION_STACK_OVERFLOW";
      case EXCEPTION_INVALID_DISPOSITION: return "EXCEPTION_INVALID_DISPOSITION";
      case EXCEPTION_GUARD_PAGE: return "EXCEPTION_GUARD_PAGE";
      case EXCEPTION_INVALID_HANDLE: return "EXCEPTION_INVALID_HANDLE";
      case CONTROL_C_EXIT: return "CONTROL_C_EXIT";

      case EXCEPTION_THROW_EXCEPTION: return "EXCEPTION_THROW_EXCEPTION";
      case EXCEPTION_INVALID_PARAMETER: return "EXCEPTION_INVALID_PARAMETER";
      case EXCEPTION_PURE_CALL: return "EXCEPTION_PURE_CALL";
      case EXCEPTION_OUT_OF_MEMORY: return "EXCEPTION_OUT_OF_MEMORY";
      case EXCEPTION_TERMINATE: return "EXCEPTION_TERMINATE";
      case EXCEPTION_UNEXPECTED: return "EXCEPTION_UNEXPECTED";

      case EXCEPTION_SIGABRT: return "EXCEPTION_SIGABRT";
      case EXCEPTION_SIGINT: return "EXCEPTION_SIGINT";
      case EXCEPTION_SIGTERM: return "EXCEPTION_SIGTERM";
      case EXCEPTION_SIGFPE: return "EXCEPTION_SIGFPE";
      case EXCEPTION_SIGILL: return "EXCEPTION_SIGILL";
      case EXCEPTION_SIGSEGV: return "EXCEPTION_SIGSEGV";

      case EXCEPTION_USER_CREATED_NORMAL: return "EXCEPTION_USER_CREATED_NORMAL";
      case EXCEPTION_USER_CREATED_FULL_DUMP: return "EXCEPTION_USER_CREATED_FULL_DUMP";
      }
      return "UNKNOWN";
    }

    static EXCEPTION_POINTERS* MakeEP(DWORD code)
    {
      CONTEXT* context = new CONTEXT;
      RtlCaptureContext(context);

      EXCEPTION_RECORD* er = new EXCEPTION_RECORD;
      memset(er, 0x00, sizeof(*er));
      er->ExceptionCode = code;
      er->ExceptionAddress = _ReturnAddress();

      EXCEPTION_POINTERS* ep = new EXCEPTION_POINTERS;
      ep->ContextRecord = context;
      ep->ExceptionRecord = er;
      return ep;
    }

    // 다른 곳에서 exception filter를 덮어쓰지 못하도록 함
    // http://blog.kalmbach-software.de/2008/04/02/unhandled-exceptions-in-vc8-and-above-for-x86-and-x64/
    // http://blog.kalmbachnet.de/?postid=75
    static BOOL PreventSetUnhandledExceptionFilter()
    {
      HMODULE hKernel32 = LoadLibraryW(L"kernel32.dll");
      if (hKernel32 == NULL) return FALSE;

      void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
      if (pOrgEntry == NULL) return FALSE;

      DWORD dwOldProtect = 0;
      SIZE_T jmpSize = 5;
#ifdef _M_X64
      jmpSize = 13;
#endif
      BOOL bProt = VirtualProtect(pOrgEntry, jmpSize,
        PAGE_EXECUTE_READWRITE, &dwOldProtect);
      BYTE newJump[20];
      void *pNewFunc = &seh_handler;
#ifdef _M_IX86
      DWORD dwOrgEntryAddr = (DWORD)pOrgEntry;
      dwOrgEntryAddr += jmpSize; // add 5 for 5 op-codes for jmp rel32
      DWORD dwNewEntryAddr = (DWORD)pNewFunc;
      DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;
      // JMP rel32: Jump near, relative, displacement relative to next instruction.
      newJump[0] = 0xE9;  // JMP rel32
      memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
#elif _M_X64
      // We must use R10 or R11, because these are "scratch" registers
      // which need not to be preserved access function calls
      // For more info see: Register Usage for x64 64-Bit
      // http://msdn.microsoft.com/en-us/library/ms794547.aspx
      // Thanks to Matthew Smith!!!
      newJump[0] = 0x49;  // MOV R11, ...
      newJump[1] = 0xBB;  // ...
      memcpy(&newJump[2], &pNewFunc, sizeof(pNewFunc));
      //pCur += sizeof (ULONG_PTR);
      newJump[10] = 0x41;  // JMP R11, ...
      newJump[11] = 0xFF;  // ...
      newJump[12] = 0xE3;  // ...
#endif
      SIZE_T bytesWritten;
      BOOL bRet = WriteProcessMemory(GetCurrentProcess(),
        pOrgEntry, newJump, jmpSize, &bytesWritten);

      if (bProt != FALSE)
      {
        DWORD dwBuf;
        VirtualProtect(pOrgEntry, jmpSize, dwOldProtect, &dwBuf);
      }
      return bRet;
    }

  };

}
