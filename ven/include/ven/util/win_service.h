#pragma once

#include <shellapi.h>

/*
** example
  class Server : public WinService::Event
  {
  public:
    void run() { WinService::run(this); }

    virtual bool on_init(int argc, wchar_t** argv) override {return true; }
    virtual bool on_running() override
    {
      Sleep(100);
      return true;
    }
    virtual void on_uninit() override {}
  };

  Server().run();

** 윈도우 서비스 등록/삭제 콘솔 명령 (관리자 권한)
  등록> sc create VEN binpath= "C:\bin\ven.exe" displayname= "VEN" start= auto type= own
  설명> sc description VEN "ven description"

  삭제> sc delete VEN

*/

namespace ven {

  class WinService
  {
  public:
    class Event
    {
    public:
      // 서비스 시작시 호출. return fasle 시 중단
      virtual bool on_init(int32_t argc, wchar_t** argv) { return true; }

      // 서비스 수행중 호출. 별도 스레드에서 while 문으로 호출되므로 Sleep 필요
      // return fasle 시 중단
      virtual bool on_running()
      {
        Sleep(100);
        return true;
      }

      // 서비스 종료시 호출
      virtual void on_uninit() {}
    };

  private:
    struct Conf
    {
      const bool has_console_ = ven::has_console();
      Event* evt_ = nullptr;
      SERVICE_STATUS_HANDLE handle_ = NULL;
      DWORD state_ = SERVICE_STOPPED;
    };

  public:
    static bool run(Event* evt)
    {
      Conf& c = conf();
      c.evt_ = evt;

      if (c.has_console_) {
        int argc = 0;
        wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        service_main(argc, argv);
      }
      else {
        SERVICE_TABLE_ENTRYW table[] = {
          { L"", (LPSERVICE_MAIN_FUNCTIONW)service_main },
          { NULL, NULL }
        };
        return (StartServiceCtrlDispatcherW(table) == FALSE);
      }

      return true;
    }


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

    static void WINAPI service_main(int32_t argc, wchar_t** argv)
    {

      Event* evt = conf().evt_;

      /*
      윈도우 서비스 실행 시
      첫번째 인자는 서비스 이름 및 별도 스레드에서 동작함
      콘솔 실행 시
      프로그램에서 받은 인자 및 main 스레드 사용
      */
      if (!conf().has_console_) {
        SERVICE_STATUS_HANDLE& handle = conf().handle_;
        handle = RegisterServiceCtrlHandlerW(L"", handler);
        if (handle == NULL) {
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
  };

}
