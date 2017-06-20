#pragma once

namespace ven {

  class IOThread
    : public Thread
    , public NetErrorHandler
  {
  private:
    IOCP& iocp_;

  public:
    IOThread(IOCP& iocp)
      : iocp_(iocp)
    {}

    virtual ~IOThread() override {}

  protected:
    virtual void run() override
    {
      uint32_t bytes = 0;
      OV* ov = nullptr;
      bool ret = false;
      uint32_t err = 0;

      while (true) {
        ret = iocp_.get(bytes, ov);
        err = (!ret) ? GetLastError() : 0;

        if (ov) {
          //err 넘겨주고 에러인경우 disconnect
          ov->actor_->on_event(err, ov, bytes);
        }
        else {
          if (ret) {
            // PostQueuedCompletionPort 활용 가능한 경우
          }
          else {
            if (err == WAIT_TIMEOUT) {
              // TIMEOUT
            }
            else if (err == ERROR_ABANDONED_WAIT_0) {
              // IOCP iocp handle closed
              // 종료할때이므로 별도 로깅 안함
              break;
            }
            else {
              net_error("io thread(IOCP unexpected error)", err, NET_FFL);
              break;
            }
          }
        }
      }
    }

  };

}
