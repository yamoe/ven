#pragma once

namespace ven {

  class Buf
  {
  public:
    byte_t* buf_ = nullptr;
    ui32_t len_ = 0;

  private:
    Mem* mem_ = nullptr;

  public:
#pragma region default
    Buf() {}

    Buf(Mem* mem)
      : mem_(mem)
    {
      if (mem_) {
        mem_->add_ref();
        buf_ = mem_->addr_;
        len_ = mem_->unit_;
      }
    }

    Buf(const Buf& cls)
      : mem_(cls.mem_)
    {
      if (mem_) {
        mem_->add_ref();
        buf_ = cls.buf_;
        len_ = cls.len_;
      }
    }

    ~Buf()
    {
      if (mem_) mem_->rel_ref();
    }

    Buf& operator=(const Buf& cls)
    {
      if (this == &cls) {
        return *this;
      }

      if (mem_) mem_->rel_ref();
      mem_ = cls.mem_;
      buf_ = cls.buf_;
      len_ = cls.len_;
      if (mem_) mem_->add_ref();
      return *this;
    }

    void swap(Buf& b)
    {
      std::swap(buf_, b.buf_);
      std::swap(len_, b.len_);
      std::swap(mem_, b.mem_);
    }

    Mem* dettach()
    {
      Mem* mem = mem_;
      mem_ = nullptr;
      buf_ = nullptr;
      len_ = 0;
      return mem;
    }

    Mem* operator->() const
    {
      return mem_;
    }

    operator bool() const
    {
      return (mem_ != nullptr);
    }

#pragma endregion
    template <class T>
    T get(ui32_t pos)
    {
      T t;
      get(t, pos);
      return t;
    }

    template <class T>
    ui32_t set(T& v, ui32_t pos)
    {
      ui32_t size = Sizer::size(v);
      if (pos + size > len_) {
        return 0;
      }
      set(v, pos, size);
      return size;
    }

#pragma region primitive, fixed array(ex. char a[10])
  public:
    template <class T>
    ui32_t get(T& v, ui32_t pos)
    {
      ui32_t size = Sizer::size(v);
      if (pos + size > len_) {
        return 0;
      }

      memcpy(&v, (buf_ + pos), size);
      return size;
    }

  private:
    template <class T>
    void set(T& v, ui32_t pos, ui32_t size)
    {
      memcpy(buf_ + pos, &v, size);
    }
#pragma endregion

#pragma region std::string
  public:
    template <>
    ui32_t get(std::string& v, ui32_t pos)
    {
      ui32_t cnt = 0;
      ui32_t size = get_cnt(pos, cnt);

      if (cnt == 0) {
        return size;
      }

      byte_t* s = buf_ + pos;
      v.assign(s, s + cnt);
      return size + cnt;
    }

  private:
    template <>
    void set(std::string& v, ui32_t pos, ui32_t size)
    {
      ui32_t cnt = static_cast<ui32_t>(v.size());
      set_cnt(pos, cnt);

      byte_t* s = buf_ + pos;
      memcpy(s, v.c_str(), cnt);
    }
#pragma endregion

#pragma region std::vector
  public:
    template <class T>
    ui32_t get(std::vector<T>& v, ui32_t pos)
    {
      ui32_t cnt = 0;
      ui32_t size = get_cnt(pos, cnt);

      for (ui32_t i = 0; i < cnt; ++i) {
        T t;
        ui32_t s = get(t, pos);
        v.push_back(t);

        size += s;
        pos += s;
      }
      return size;
    }

  private:
    template <class T>
    void set(std::vector<T>& v, ui32_t pos, ui32_t size)
    {
      ui32_t cnt = v.size();
      set_cnt(pos, cnt);

      for (auto& t : v) {
        pos += set(const_cast<T&>(t), pos);
      }
    }
#pragma endregion

#pragma region std::set
  public:
    template <class T>
    ui32_t get(std::set<T>& v, ui32_t pos)
    {
      ui32_t cnt = 0;
      ui32_t size = get_cnt(pos, cnt);

      for (ui32_t i = 0; i < cnt; ++i) {
        T t;
        ui32_t s = get(t, pos);
        v.insert(t);

        size += s;
        pos += s;
      }
      return size;
    }

  private:
    template <class T>
    void set(std::set<T>& v, ui32_t pos, ui32_t size)
    {
      ui32_t cnt = v.size();
      set_cnt(pos, cnt);

      for (auto& t : v) {
        pos += set(const_cast<T&>(t), pos);
      }
    }
#pragma endregion

#pragma region std::unordered_set
  public:
    template <class T>
    ui32_t get(std::unordered_set<T>& v, ui32_t pos)
    {
      ui32_t cnt = 0;
      ui32_t size = get_cnt(pos, cnt);

      for (ui32_t i = 0; i < cnt; ++i) {
        T t;
        ui32_t s = get(t, pos);
        v.insert(t);

        size += s;
        pos += s;
      }
      return size;
    }

  private:
    template <class T>
    void set(std::unordered_set<T>& v, ui32_t pos, ui32_t size)
    {
      ui32_t cnt = static_cast<ui32_t>(v.size());
      set_cnt(pos, cnt);

      for (auto& t : v) {
        pos += set(const_cast<T&>(t), pos);
      }
    }
#pragma endregion

#pragma region std::map
  public:
    template <class T1, class T2>
    ui32_t get(std::map<T1, T2>& v, ui32_t pos)
    {
      ui32_t cnt = 0;
      ui32_t size = get_cnt(pos, cnt);

      for (ui32_t i = 0; i < cnt; ++i) {
        ui32_t s = 0;
        T1 t1;
        s = get(t1, pos);
        size += s;
        pos += s;

        T2 t2;
        s = get(t2, pos);
        size += s;
        pos += s;

        v.insert(std::make_pair(t1, t2));
      }
      return size;
    }

  private:
    template <class T1, class T2>
    void set(std::map<T1, T2>& v, ui32_t pos, ui32_t size)
    {
      ui32_t cnt = v.size();
      set_cnt(pos, cnt);

      for (auto& kv : v) {
        pos += set(const_cast<T1&>(kv.first), pos);
        pos += set(const_cast<T2&>(kv.second), pos);
      }
    }

#pragma endregion

#pragma region std::unordered_map
  public:
    template <class T1, class T2>
    ui32_t get(std::unordered_map<T1, T2>& v, ui32_t pos)
    {
      ui32_t cnt = 0;
      ui32_t size = get_cnt(pos, cnt);

      for (ui32_t i = 0; i < cnt; ++i) {
        ui32_t s = 0;
        T1 t1;
        s = get(t1, pos);
        size += s;
        pos += s;

        T2 t2;
        s = get(t2, pos);
        size += s;
        pos += s;

        v.insert(std::make_pair(t1, t2));
      }
      return size;
    }

  private:
    template <class T1, class T2>
    void set(std::unordered_map<T1, T2>& v, ui32_t pos, ui32_t size)
    {
      ui32_t cnt = static_cast<ui32_t>(v.size());
      set_cnt(pos, cnt);

      for (auto& kv : v) {
        pos += set(const_cast<T1&>(kv.first), pos);
        pos += set(const_cast<T2&>(kv.second), pos);
      }
    }

#pragma endregion

  private:
    ui32_t get_cnt(ui32_t& pos, ui32_t& cnt)
    {
      ui32_t size = get(cnt, pos);
      pos += size;
      return size;
    }

    void set_cnt(ui32_t& pos, ui32_t cnt)
    {
      set(cnt, pos, Sizer::cnt_size());
      pos += Sizer::cnt_size();
    }


  };

}
