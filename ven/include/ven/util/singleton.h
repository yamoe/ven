#pragma once

namespace ven {

  template <class T>
  class Singleton : public NonCopyable {
  private:
    static T* inst_;

  public:
    Singleton() {}
    virtual ~Singleton() {}

    static T& inst()
    {
      if (!inst_) inst_ = new T;
      return *inst_;
    }

    static void uninst()
    {
      if (!inst_) return;
      delete inst_;
      inst_ = nullptr;
    }

  };

  template <class T> __declspec(selectany)  T * Singleton<T>::inst_ = nullptr;

}
