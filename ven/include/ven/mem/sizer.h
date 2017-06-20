#pragma once

namespace ven {

  class Sizer
  {
  public:
    static uint32_t cnt_size()
    {
      return 4;
    }

    template <class T>
    static uint32_t size(T& v)
    {
      return sizeof(v);
    }

    template <class T, uint32_t cnt>
    static uint32_t size(T(&arr)[cnt])
    {
      return size(arr[0]) * cnt;
    }

    template <>
    static uint32_t size(std::string& v)
    {
      return cnt_size() + static_cast<uint32_t>(v.size());
    }

    template <class T>
    static uint32_t size(std::vector<T>& v)
    {
      uint32_t size = cnt_size();
      for (auto& t : v) {
        size += Sizer::size(const_cast<T&>(t));
      }
      return size;
    }

    template <class T>
    static uint32_t size(std::set<T>& v)
    {
      uint32_t size = cnt_size();
      for (auto& t : v) {
        size += Sizer::size(const_cast<T&>(t));
      }
      return size;
    }

    template <class T>
    static uint32_t size(std::unordered_set<T>& v)
    {
      uint32_t size = cnt_size();
      for (auto& t : v) {
        size += Sizer::size(const_cast<T&>(t));
      }
      return size;
    }

    template <class T1, class T2>
    static uint32_t size(std::map<T1, T2>& v)
    {
      uint32_t size = cnt_size();
      for (auto& kv : v) {
        size += Sizer::size(const_cast<T1&>(kv.first));
        size += Sizer::size(const_cast<T2&>(kv.second));
      }
      return size;
    }

    template <class T1, class T2>
    static uint32_t size(std::unordered_map<T1, T2>& v)
    {
      uint32_t size = cnt_size();
      for (auto& kv : v) {
        size += Sizer::size(const_cast<T1&>(kv.first));
        size += Sizer::size(const_cast<T2&>(kv.second));
      }
      return size;
    }
  };
}