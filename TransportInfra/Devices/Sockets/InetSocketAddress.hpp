#ifndef INETSOCKETADDRESS_HPP
#define INETSOCKETADDRESS_HPP
//#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <cstring>

#include <memory>

#include "Traits/socket_traits.hpp"
#include "HostAddress.hpp"

namespace infra
{

namespace inet
{

template<typename HostAddress>
struct NetworkAddress{
    NetworkAddress() = default;
    NetworkAddress(const HostAddress & host_addr, uint16_t port) :
        port_number{port},
        host_address{host_addr}
    {}

    using inet_domain = typename HostAddress::inet_domain;

    uint16_t    port_number{0};
    HostAddress host_address;

    bool empty() const { return 0 == port_number && host_address.toStringReadOnly().empty(); }

    bool operator==(const NetworkAddress & other) const{
        return host_address == other.host_address && port_number == other.port_number; }

    bool operator<(const NetworkAddress & other) const{
         return (host_address < other.host_address) ?
                     true : (host_address == other.host_address) ?
                         (port_number < other.port_number) : false; }
};

template<typename NetworkAddress, typename = void>
class InetSocketAddress{};


template<typename NetworkAddress>
class InetSocketAddress<NetworkAddress, std::enable_if_t<IsIPV4NetworkAddress<NetworkAddress>::value>>
{
public:
    using address_type = sockaddr_in;

public:
    InetSocketAddress() :
        m_address{},
        m_p_in_addr{std::make_unique<sockaddr_in>()}
    {
        memset(m_p_in_addr.get(), 0, getAddressLength());
        m_p_in_addr->sin_family = PF_INET;
    }

    InetSocketAddress(NetworkAddress addr) :
        m_address{},
        m_p_in_addr{std::make_unique<sockaddr_in>()}
    {
        setAddress(addr);
    }

    InetSocketAddress(const InetSocketAddress & other) :
        m_address{other.m_address},
        m_p_in_addr{std::make_unique<sockaddr_in>()}
    {
        memset(m_p_in_addr.get(), 0, getAddressLength());
        m_p_in_addr->sin_port = other.m_p_in_addr->sin_port;
        m_p_in_addr->sin_addr.s_addr = other.m_p_in_addr->sin_addr.s_addr;
        m_p_in_addr->sin_family = PF_INET;
    }


    InetSocketAddress & operator=(const InetSocketAddress & other) {
        if (this == &other) return *this;
        m_address = other.m_address;
        memset(m_p_in_addr.get(), 0, getAddressLength());
        m_p_in_addr->sin_port = other.m_p_in_addr->sin_port;
        m_p_in_addr->sin_addr.s_addr = other.m_p_in_addr->sin_addr.s_addr;
        m_p_in_addr->sin_family = PF_INET;
        return *this;
    }

    InetSocketAddress(InetSocketAddress && other) = default;
    InetSocketAddress & operator=(InetSocketAddress && other) = default;
    ~InetSocketAddress() = default;

    bool setAddress(NetworkAddress addr){
        if (addr.host_address.isValid()){
            m_address = std::move(addr);
            m_p_in_addr->sin_port = htons(addr.port_number);
            m_p_in_addr->sin_addr.s_addr = htonl(addr.host_address.getAddress());
            return true;
        }
        return false;
    }

    void setAddress(const sockaddr &sock_addr){
        const sockaddr_in *p_remote_ipv4_sock_addr = reinterpret_cast<const sockaddr_in*>(&sock_addr);
        memset(m_p_in_addr.get(), 0, getAddressLength());
        m_p_in_addr->sin_family = PF_INET;
        m_p_in_addr->sin_port = p_remote_ipv4_sock_addr->sin_port;
        m_p_in_addr->sin_addr.s_addr = p_remote_ipv4_sock_addr->sin_addr.s_addr;
        m_address.port_number = ntohs(m_p_in_addr->sin_port);
        in_addr host_in_addr;
        host_in_addr.s_addr = ntohl(m_p_in_addr->sin_addr.s_addr);
        m_address.host_address.setAddress(host_in_addr);
    }

    void getAddress(address_type & out_addr) const{
         out_addr.sin_port = m_p_in_addr->sin_port;
         out_addr.sin_family = PF_INET;
         out_addr.sin_addr.s_addr = m_p_in_addr->sin_addr.s_addr;
    }

    void getAddress(sockaddr & out_addr) const{
         sockaddr_in *p_out_in_addr = reinterpret_cast<sockaddr_in*>(&out_addr);
         p_out_in_addr->sin_family = PF_INET;
         p_out_in_addr->sin_port = m_p_in_addr->sin_port;
         p_out_in_addr->sin_addr.s_addr = m_p_in_addr->sin_addr.s_addr;
    }

    bool empty() const { return m_address.empty(); }

    template<typename U>
    friend bool operator<(const InetSocketAddress<U> & left, const InetSocketAddress<U> & right);

public:
    inline static size_t getAddressLength(){
        static constexpr auto size{sizeof (sockaddr_in)};
        return size;
    }

private:
    NetworkAddress m_address;
    std::unique_ptr<sockaddr_in> m_p_in_addr;
};


template<typename NetworkAddress>
class InetSocketAddress<NetworkAddress, std::enable_if_t<IsIPV6NetworkAddress<NetworkAddress>::value>>
{
public:
    using address_type = sockaddr_in6;

public:
    InetSocketAddress() :
        m_address{},
        m_p_in6_addr{std::make_unique<sockaddr_in6>()}
    {
        memset(m_p_in6_addr.get(), 0, getAddressLength());
        m_p_in6_addr->sin6_family = PF_INET6;
        m_p_in6_addr->sin6_port = 0;
    }

    InetSocketAddress(const InetSocketAddress & other) :
        m_address{other.m_address},
        m_p_in6_addr{std::make_unique<sockaddr_in6>()}
    {
        memset(m_p_in6_addr.get(), 0, getAddressLength());
        m_p_in6_addr->sin6_port = other.m_p_in6_addr->sin6_port;
        m_p_in6_addr->sin6_family = PF_INET6;

        const uint8_t *p_remote_ip_addr = reinterpret_cast<const uint8_t*>(&other.m_p_in6_addr->sin6_addr.__in6_u.__u6_addr8);
        uint8_t *p_local_ip_addr = reinterpret_cast<uint8_t*>(&m_p_in6_addr->sin6_addr);
        std::copy(p_remote_ip_addr, p_remote_ip_addr + IPV6_ADDR_SIZE, p_local_ip_addr);
    }


    InetSocketAddress & operator=(const InetSocketAddress & other) {
        if (this == &other) return *this;
        m_address = other.m_address;
        memset(m_p_in6_addr.get(), 0, getAddressLength());
        m_p_in6_addr->sin6_port = other.m_p_in6_addr->sin6_port;
        m_p_in6_addr->sin6_family = PF_INET6;

        const uint8_t *p_remote_ip_addr = reinterpret_cast<const uint8_t*>(&other.m_p_in6_addr->sin6_addr.__in6_u.__u6_addr8);
        uint8_t *p_local_ip_addr = reinterpret_cast<uint8_t*>(&m_p_in6_addr->sin6_addr);
        std::copy(p_remote_ip_addr, p_remote_ip_addr + IPV6_ADDR_SIZE, p_local_ip_addr);
        return *this;
    }

    InetSocketAddress(InetSocketAddress && other) = default;

    InetSocketAddress & operator=(InetSocketAddress && other) = default;
    ~InetSocketAddress() = default;


public:
    bool setAddress(NetworkAddress addr){
          if (!addr.host_address.isValid() || addr.port_number <= 0) return false;

          m_p_in6_addr->sin6_port = htons(addr.port_number);
         // return addr.host_address.getAddress(m_p_in6_addr->sin6_addr.__in6_u);
          return addr.host_address.getAddress(m_p_in6_addr->sin6_addr.__in6_u.__u6_addr8);
    }

    void setAddress(const sockaddr &sock_addr){
        const sockaddr_in6 *p_remote_ipv6_sock_addr = reinterpret_cast<const sockaddr_in6*>(&sock_addr);
        memset(m_p_in6_addr.get(), 0, getAddressLength());
        m_p_in6_addr->sin6_family = PF_INET6;
        m_p_in6_addr->sin6_port = p_remote_ipv6_sock_addr->sin6_port;

        const uint8_t *p_remote_ip_addr = reinterpret_cast<const uint8_t*>(&p_remote_ipv6_sock_addr->sin6_addr.__in6_u.__u6_addr8);
        uint8_t *p_local_ip_addr = reinterpret_cast<uint8_t*>(&m_p_in6_addr->sin6_addr);
        std::copy(p_remote_ip_addr, p_remote_ip_addr + IPV6_ADDR_SIZE, p_local_ip_addr);

        m_address.port_number = ntohs(m_p_in6_addr->sin6_port);
        m_address.host_address.setAddress(m_p_in6_addr->sin6_addr);
    }

    void getAddress(address_type & out_addr) const{
         out_addr.sin6_port = m_p_in6_addr->sin6_port;
         out_addr.sin6_family = PF_INET6;

         uint8_t *p_remote_ip_addr = reinterpret_cast<uint8_t*>(&out_addr.sin6_addr.__in6_u);
         const uint8_t *p_local_ip_addr = reinterpret_cast<const uint8_t*>(&m_p_in6_addr->sin6_addr);
         std::copy(p_local_ip_addr, p_local_ip_addr + IPV6_ADDR_SIZE, p_remote_ip_addr);
    }

    void getAddress(sockaddr & out_addr) const{
        sockaddr_in6 *p_out_in6_addr = reinterpret_cast<sockaddr_in6*>(&out_addr);
        p_out_in6_addr->sin6_port = m_p_in6_addr->sin6_port;
        p_out_in6_addr->sin6_family = PF_INET6;

        uint8_t *p_remote_ip_addr = reinterpret_cast<uint8_t*>(&p_out_in6_addr->sin6_addr.__in6_u);
        const uint8_t *p_local_ip_addr = reinterpret_cast<const uint8_t*>(&m_p_in6_addr->sin6_addr);
        std::copy(p_local_ip_addr, p_local_ip_addr + IPV6_ADDR_SIZE, p_remote_ip_addr);

    }

    bool empty() const { return m_address.empty(); }

    template<typename U>
    friend bool operator<(const InetSocketAddress<U> & left, const InetSocketAddress<U> & right);

public:
    inline static size_t getAddressLength(){
        static constexpr auto size{sizeof (sockaddr_in6)};
        return size;
    }

private:
    static inline constexpr size_t IPV6_ADDR_SIZE{16};

private:
    NetworkAddress m_address;
    std::unique_ptr<sockaddr_in6> m_p_in6_addr;
};


template<typename NetworkAddress>
bool operator<(const InetSocketAddress<NetworkAddress> & left, const InetSocketAddress<NetworkAddress> & right){
    return left.m_address < right.m_address;
}

using IPV4NetworkAddress = NetworkAddress<IPV4HostAddr>;
using IPV4InetSocketAddress = InetSocketAddress<IPV4NetworkAddress>;

} //inet

} //infra

#endif // INETSOCKETADDRESS_HPP
