#pragma once

namespace ven {

  class RBuf {
  private:
    ui32_t default_size_ = 0;
    ui32_t remaining_size_to_exchange_ = 0;

    Buf buf_;
    byte_t* head_ = nullptr;
    byte_t* tail_ = nullptr;
    IMemPool* mpool_ = nullptr;

  public:
    RBuf() {}

    ~RBuf() { uninit(); }

    byte_t* head() { return head_; }

    byte_t* tail() { return tail_; }

    ui32_t data_len() { return static_cast<ui32_t>(tail_ - head_); }

    ui32_t remain_len() { return static_cast<ui32_t>((buf_->addr_ + buf_->unit_) - tail_); }

    ui32_t use_len() { return static_cast<ui32_t>(tail_ - buf_->addr_); }

    void move_tail(ui32_t dist) { tail_ += dist; }

    void move_head(ui32_t dist) { head_ += dist; }

    Buf copy_buf() { return buf_; }

    void check_size(ui32_t packet_size)
    {
      // 패킷이 큰 경우
      if (buf_->unit_ < packet_size) {
        change(packet_size * 2);
        return;
      }
    }

    void check_arrange()
    {
      // 줄일 수 있는 경우
      if ((default_size_ < buf_->unit_) && (data_len() < default_size_)) {
        change(default_size_);
        return;
      }

      // 거의 가득 찬 경우 교체
      if (remain_len() <= remaining_size_to_exchange_) {
        change(buf_->unit_);
        return;
      }
    }

    void init(
      IMemPool* mpool,
      ui32_t buffer_size,
      ui32_t remaining_size_to_exchange
    )
    {
      mpool_ = mpool;
      default_size_ = buffer_size;
      remaining_size_to_exchange_ = remaining_size_to_exchange;

      buf_ = mpool_->get(default_size_);
      head_ = buf_->addr_;
      tail_ = head_;
    }

    void uninit()
    {
      buf_ = Buf();
      head_ = nullptr;
      tail_ = nullptr;
    }

  private:
    void change(ui32_t size)
    {
      Buf buf = mpool_->get(size);

      ui32_t len = data_len();
      if (len > 0) {
        memcpy(buf->addr_, head_, len);
      }

      head_ = buf->addr_;
      tail_ = head_ + len;
      buf_ = buf;
    }

  };

}
