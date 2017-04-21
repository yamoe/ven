#pragma once

namespace ven {

  template <class T>
  class Auto {
  private:
    T* t_ = nullptr;
    bool is_mine_ = true;

  public:
    Auto() {}

    Auto(T* t) : t_(t) {}

    ~Auto()
    {
      del();
    }

    void not_mine()
    {
      is_mine_ = false;
    }

    T* operator->()
    {
      return t_;
    }

    void operator=(T* t)
    {
      (*this)(t);
    }

    void operator()(T* t)
    {
      del();
      t_ = t;
    }

    T& operator*()
    {
      return *t_;
    }

    void reset(T* t = nullptr)
    {
      del();
      t_ = t;
    }

    operator bool()
    {
      return (t_ != nullptr);
    }

    operator T*()
    {
      return t_;
    }

  private:
    void del()
    {
      if (is_mine_) {
        delete t_;
      }
      t_ = nullptr;
    }

  };


  template <class Key, class Value>
  class AutoMap {
  private:
    typedef std::function<void(Value&)> DelFunc;
    typedef std::unordered_map<Key, Value> Container;
    Container cont_;
    SLock lock_;

    DelFunc del_func_ = [](Value& v) {delete v; };

  public:
    AutoMap() {}

    ~AutoMap()
    {
      SLocker lock(lock_);
      for (auto& it : cont_) {
        del_func_(it.second);
      }
    }

    void add(const Key& k, const Value& v)
    {
      SLocker lock(lock_);
      cont_.emplace(k, v);
    }
  };


  template <class Value>
  class AutoVec {
  private:
    typedef std::function<void(Value&)> DelFunc;
    typedef std::vector<Value> Container;
    Container cont_;
    SLock lock_;

    DelFunc del_func_ = [](Value& v) {delete v; };

  public:
    AutoVec() {}

    ~AutoVec()
    {
      SLocker lock(lock_);
      for (auto& it : cont_) {
        del_func_(it);
      }
    }

    void add(const Value& v)
    {
      SLocker lock(lock_);
      cont_.push_back(v);
    }

  };

}