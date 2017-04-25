#pragma once

namespace ven
{

  class Addr
  {
  public:
    ip_t ip_ = { 0, };
    port_t port_ = 0;

  public:
    Addr() {}

    Addr(ip_t& ip, port_t port)
    {
      copy(ip, ip_);
      port_ = port;
    }

    Addr(cchar_t* ip, port_t port)
    {
      copy(ip, ip_);
      port_ = port;
    }

    Addr(const std::string& ip, port_t port)
    {
      copy(ip.c_str(), ip_);
      port_ = port;
    }

    Addr(const Addr& addr)
    {
      copy(addr.ip_, ip_);
      port_ = addr.port_;
    }

    Addr(const sockaddr_in& sockaddr)
    {
      set_sockaddr(sockaddr);
    }

    Addr& operator=(const Addr& addr)
    {
      if (this == &addr) {
        return *this;
      }

      copy(addr.ip_, ip_);
      port_ = addr.port_;

      return *this;
    }

    Addr& operator=(const sockaddr_in& sockaddr)
    {
      set_sockaddr(sockaddr);
      return *this;
    }

    bool operator<(const Addr& addr) const
    {
      int_t r = strcmp(ip_, addr.ip_);
      if (r == 0) {
        return (port_ < addr.port_);
      }
      return (r < 0);
    }

    operator bool() const
    {
      return (port_ != 0);
    }

    bool is_any_ip() const
    {
      return (ip_[0] == 0);
    }

    sockaddr_in sockaddr() const
    {
      sockaddr_in addr;
      memset(&addr, 0x00, sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_port = htons(port_);

      if (is_any_ip()) {
        addr.sin_addr.s_addr = INADDR_ANY;
        
      } else {
        inet_pton(AF_INET, ip_, &addr.sin_addr.s_addr);
      }
      return addr;
    }

    void set_sockaddr(const sockaddr_in& addr)
    {
      inet_ntop(AF_INET, const_cast<in_addr*>(&addr.sin_addr), ip_, sizeof(ip_));
      port_ = ntohs(addr.sin_port);
    }

  };


  struct AddrHash
  {
    std::size_t operator() (const Addr& addr) const {
      return std::hash<std::size_t>()(
        std::hash<const char*>()(addr.ip_)
        + addr.port_
        );
    }
  };


  struct AddrEqualTo
  {
    bool operator() (const Addr& a1, const Addr& a2) const
    {
      return (a1.port_ == a2.port_) && (strcmp(a1.ip_, a2.ip_) == 0);
    }
  };


  template <class Key>
  using AddrUnorderedMap = std::unordered_map<Addr, Key, AddrHash, AddrEqualTo>;

  template <class Key>
  using AddrMap = std::map<Addr, Key>;

}
