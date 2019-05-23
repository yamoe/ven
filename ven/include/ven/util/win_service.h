#pragma once

/*
** description
  콘솔 실행 시 - 프로그램에서 받은 인자 및 main 스레드 사용
  윈도우 서비스 실행 시 - 첫번째 인자는 서비스 이름. on_runing은 별도 스레드.

** example
  class Server : public ven::WinService::Event
  {
  public:
    virtual bool on_init(int argc, char** argv) override { return true; }
    virtual bool on_running() override
    {
    ven::sleep(100);
    return true;
    }
    virtual void on_uninit() override {}
  };

  int main(int argc, char* argv[])
  {
    Server svr;
    return ven::WinService::run(svr, argc, argv) ? 0 : -1;
  }

** 윈도우 서비스 등록/삭제 콘솔 명령 (관리자 권한)
  등록> sc create VEN binpath= "C:\bin\ven.exe" displayname= "VEN" start= auto type= own
  설명> sc description VEN "ven description"
  삭제> sc delete VEN

*/

#ifndef _WINSVC_
#include <winsvc.h>
#endif

namespace ven {

  class WinService
  {
  public:
    class Event
    {
    public:
      // 서비스 시작시 호출. return fasle 시 중단
      virtual bool on_init(int32_t argc, char** argv) { return true; }

      // 서비스 수행중 호출. 별도 스레드에서 while 문으로 호출되므로 Sleep 필요
      // return fasle 시 중단
      virtual bool on_running()
      {
        ven::sleep(100);
        return true;
      }

      // 서비스 종료시 호출
      virtual void on_uninit() {}
    };

  private:
#if defined(WINDOWS)
    class Conf
    {
    public:
      Event* evt_ = nullptr;
      int argc_ = 0;
      char** argv_ = nullptr;

      const bool has_console_ = ven::has_console();

      SERVICE_STATUS_HANDLE handle_ = NULL;
      DWORD state_ = SERVICE_STOPPED;
    };
#endif

  public:
    static bool run(Event& evt, int argc = 0, char** argv = nullptr)
    {
#if defined(WINDOWS)
      Conf& c = conf();
      c.evt_ = &evt;
      c.argc_ = argc;
      c.argv_ = argv;

      if (c.has_console_) {
        service_main(c.argc_, c.argv_);
      }
      else {
        SERVICE_TABLE_ENTRYA table[] = {
          { "", (LPSERVICE_MAIN_FUNCTIONA)service_main },
          { NULL, NULL }
        };
        return (StartServiceCtrlDispatcherA(table) == FALSE);
      }
#else
      // init
      if (!evt.on_init(argc, argv)) {
        evt.on_uninit();
        return false;
      }

      // running
      bool has_console = ven::has_console();
      while (evt.on_running()) {
        if (has_console) {

          int ch = ven::linux_kbhit();
          if (ch == 27) { //ESC
            break;
          }
        }
      }

      // uninit
      evt.on_uninit();
#endif

      return true;
    }

#if defined(WINDOWS)
  private:
    static Conf& conf()
    {
      static Conf conf_;
      return conf_;
    }

    static void set_state(DWORD state)
    {
      if (!conf().has_console_) {
        SERVICE_STATUS s = { 0 };
        s.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        s.dwControlsAccepted = SERVICE_ACCEPT_STOP;
        s.dwCurrentState = state;
        s.dwWin32ExitCode = 0;
        s.dwServiceSpecificExitCode = 0;
        s.dwCheckPoint = 0;

        SetServiceStatus(conf().handle_, &s);
      }
      conf().state_ = state;
    }

    static void WINAPI service_main(int32_t argc, char** argv)
    {

      Event* evt = conf().evt_;

      if (!conf().has_console_) {
        SERVICE_STATUS_HANDLE& handle = conf().handle_;
        handle = RegisterServiceCtrlHandlerW(L"", handler);
        if (handle == nullptr) {
          evt->on_uninit();
          return;
        }
      }
      else {
        printf("Press ESC to exit\n");
      }


      // init
      set_state(SERVICE_START_PENDING);
      if (!evt->on_init(argc, argv)) {
        evt->on_uninit();
        return;
      }

      // running
      set_state(SERVICE_RUNNING);
      while (conf().state_ == SERVICE_RUNNING) {
        if (!evt->on_running()) {
          break;
        }

        if (conf().has_console_) {
          if (_kbhit() > 0) {
            int ch = _getch();
            if (ch == 27) { //ESC
              break;
            }
          }
        }
      }

      // uninit
      evt->on_uninit();
      set_state(SERVICE_STOPPED);
    }

    static void WINAPI handler(DWORD control)
    {
      if (conf().state_ == control) {
        return;
      }

      switch (control) {
      case SERVICE_CONTROL_STOP:
        set_state(SERVICE_STOP_PENDING);
        set_state(SERVICE_STOPPED);
        break;

      case SERVICE_CONTROL_INTERROGATE:
      default:
        set_state(conf().state_);
        break;
      }
    }
#endif
  };

}
