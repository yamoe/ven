#pragma once

namespace ven {

  class RBuf {
  private:
    uint32_t default_size_ = 0;
    uint32_t remaining_size_to_exchange_ = 0;

    Buf buf_;
    uint8_t* head_ = nullptr;
    uint8_t* tail_ = nullptr;
    IMemPool* mpool_ = nullptr;

  public:
    RBuf() {}

    ~RBuf() { uninit(); }

    uint8_t* head() { return head_; }

    uint8_t* tail() { return tail_; }

    uint32_t data_len() { return static_cast<uint32_t>(tail_ - head_); }

    uint32_t remain_len() { return static_cast<uint32_t>((buf_->addr_ + buf_->unit_) - tail_); }

    uint32_t use_len() { return static_cast<uint32_t>(tail_ - buf_->addr_); }

    void move_tail(uint32_t dist) { tail_ += dist; }

    void move_head(uint32_t dist) { head_ += dist; }

    Buf copy_buf() { return buf_; }

    void check_size(uint32_t packet_size)
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
      uint32_t buffer_size,
      uint32_t remaining_size_to_exchange
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
    void change(uint32_t size)
    {
      Buf buf = mpool_->get(size);

      uint32_t len = data_len();
      if (len > 0) {
        memcpy(buf->addr_, head_, len);
      }

      head_ = buf->addr_;
      tail_ = head_ + len;
      buf_ = buf;
    }

  };

}
