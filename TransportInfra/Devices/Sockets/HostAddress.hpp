#ifndef HOSTADDRESS_HPP
#define HOSTADDRESS_HPP
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>

#include <string_view>
#include <sstream>
#include <utility>
#include <array>

#include "sys_call_eval.h"

#include "Traits/socket_traits.hpp"
#include "InetUtils/InetUtilities.h"


namespace infra
{

namespace inet
{
template<typename InetDomain>
class HostAddress{};

template<>
class HostAddress<ipv4_domain>
{
public:
    using inet_domain = ipv4_domain;

    template<typename U>
    friend bool operator==(const HostAddress<U> & first, const HostAddress<U> & second);


public:
    HostAddress(){}
    HostAddress(uint32_t ipv4_addr){
        initData(ipv4_addr);
    }

    HostAddress(std::string_view address) {
        initData(address);
    }

    HostAddress(const in_addr & ipv4_addr) {
        initData(ipv4_addr.s_addr);
    }

    bool operator==(const HostAddress & other) const{
        return this->toStringReadOnly() == other.toStringReadOnly();
    }

    bool operator==(const in_addr & ipv4_addr) const{
        return this->getAddress() == ipv4_addr.s_addr;
    }

    bool operator!=(const HostAddress & other) const{
        return !(*this == other);
    }

    bool operator<(const HostAddress & other) const{
        return this->toStringReadOnly() < other.toStringReadOnly();
    }

public:
    static HostAddress LocalHost() { return HostAddress{INADDR_LOOPBACK}; }
    static HostAddress AddressAny() { return HostAddress{INADDR_ANY}; }

    static inline constexpr size_t IPV4_ADDR_SIZE{4};

public:
   void setAddressAny() {
       if (m_valid) resetData();
       initData(INADDR_ANY);
   }

   void setLocalHost() {
       if (m_valid) resetData();
       initData(ntohl(INADDR_LOOPBACK));
   }

   void setAddress(uint32_t ipv4_addr) {
       if (m_valid) resetData();
       initData(ipv4_addr);
   }

   void setAddress(std::string_view address) {
       if (m_valid) resetData();
       initData(address);
   }

   void setAddress(const in_addr & ipv4_addr) {
       if (m_valid) resetData();
       initData(ipv4_addr.s_addr);
   }

   inline void clear() { resetData(); }

public:
   bool isLoopback() const {
      return *this  == in_addr{ntohl(INADDR_LOOPBACK)};
   }

   bool isAny() const {
       return *this == in_addr{ntohl(INADDR_ANY)};
   }

   bool isValid() const { return m_valid; }

   bool getAddress(in_addr *p_ipv4_addr) const{
       if (uint32_t ipv4_addr{getAddress()}; 0 != ipv4_addr){
           p_ipv4_addr->s_addr = ipv4_addr;
           return true;
       }
       return false;
   }

   uint32_t getAddress() const{
       uint32_t ret{0};
       if (m_valid) std::copy(m_ipv4_address.begin(), m_ipv4_address.end(),
                              reinterpret_cast<uint8_t*>(&ret));
       return ret;
   }

   static size_t getAddressLength() { return IPV4_ADDR_SIZE; }

   std::string_view toStringReadOnly() const{
       return m_valid ? m_printable_address : std::string_view();
   }

   std::string toString() const{
        return m_valid ? m_printable_address : std::string();
   }

   std::string toStringFullFormat() const{
       return utils::inet::addressToString(m_ipv4_address.data(), m_ipv4_address.size());
   }

private:
   void initData(std::string_view address){
        if(extractIPv4Address(address)){
            m_printable_address = address;
            m_valid = true;
        }
   }

   void initData(uint32_t ipv4_addr){
       if (extractPrintableAddress(&ipv4_addr)){
           uint8_t *p_ipv4_addr = reinterpret_cast<uint8_t *>(&ipv4_addr);
           std::copy(p_ipv4_addr, p_ipv4_addr + IPV4_ADDR_SIZE, m_ipv4_address.begin());
           m_valid = true;
       }
   }

   void resetData(){
       m_ipv4_address.fill(0);
       m_printable_address.clear();
       m_valid = false;
   }

   bool extractIPv4Address(std::string_view address){
       void *p_addr = reinterpret_cast<void*>(&m_ipv4_address);
       return sys_call_noerr_eval(::inet_pton, PF_INET, address.data(), p_addr);
   }

   bool extractPrintableAddress(const void * src){
       char dest[INET_ADDRSTRLEN];
       memset(dest, 0, INET_ADDRSTRLEN);
       m_printable_address.reserve(INET_ADDRSTRLEN);
       if (nullptr != inet_ntop(PF_INET, src, dest, INET_ADDRSTRLEN)){
           m_printable_address.assign(dest, strnlen(dest, INET_ADDRSTRLEN));
           return true;
       }
       return false;
   }

private:
   std::array<uint8_t, 4> m_ipv4_address;
   std::string m_printable_address{};
   bool m_valid{false};
};


template<>
class HostAddress<ipv6_domain>
{

public:
    using inet_domain = ipv6_domain;

public:
    HostAddress() {}
    HostAddress(const uint8_t *p_ipv6_addr){
        initData(p_ipv6_addr);
    }

    HostAddress(const in6_addr &ipv6_addr){
        initData(reinterpret_cast<const uint8_t*>(&ipv6_addr));
    }

    HostAddress(std::string_view address){
        initData(address);
    }

    bool operator==(const HostAddress & other) const{
        return this->toStringReadOnly() == other.toStringReadOnly();
    }

    bool operator==(const in6_addr & ipv6_addr) const{
        return std::equal(m_ipv6_address.begin(), m_ipv6_address.end(),
                          reinterpret_cast<const uint8_t*>(&ipv6_addr));
    }

    bool operator!=(const HostAddress & other) const{
        return !(*this == other);
    }

    bool operator<(const HostAddress & other) const{
        return this->toStringReadOnly() < other.toStringReadOnly();
    }


public:
   static HostAddress LocalHost() { return HostAddress{reinterpret_cast<const uint8_t*>(&in6addr_loopback)}; }
   static HostAddress AddressAny() { return HostAddress{reinterpret_cast<const uint8_t*>(&in6addr_any)}; }

   static inline constexpr size_t IPV6_ADDR_SIZE{16};

public:
    void setAddressAny() {
        if (m_valid) resetData();
        initData(reinterpret_cast<const uint8_t*>(&in6addr_any));
    }
    void setLocalHost() {
        if (m_valid) resetData();
        initData(reinterpret_cast<const uint8_t*>(&in6addr_loopback));
    }

    void setAddress(const uint8_t *p_ipv6_addr){
        if (m_valid) resetData();
        initData(p_ipv6_addr);
    }

    void setAddress(std::string_view address){
        if (m_valid) resetData();
        initData(address);
    }

    void setAddress(const in6_addr & ipv6_addr){
        if (m_valid) resetData();
        initData(reinterpret_cast<const uint8_t*>(&ipv6_addr));
    }

   inline void clear() { resetData(); }

public:
    bool isLoopback() const {
        return *this == in6addr_loopback;
    }

    bool isAny() const {
        return *this == in6addr_any;
    }

    bool isValid() const {return m_valid;}

    template<size_t size>
    bool getAddress(uint8_t (&ipv6_addr)[size]) const {
        if (m_valid && IPV6_ADDR_SIZE <= size){
            std::copy(m_ipv6_address.begin(), m_ipv6_address.end(), ipv6_addr);
            return true;
        }
        return false;
    }

    //size_t getAddressSize() { return IPV6_ADDR_SIZE; }

    std::string_view toStringReadOnly() const{
        return m_valid ? m_printable_address : std::string_view();
    }

    std::string toString() const{
        return m_valid ? m_printable_address : std::string();
    }

    std::string toStringFullFormat() const{
        return utils::inet::addressToString(m_ipv6_address.data(), m_ipv6_address.size());
    }

    static std::size_t getAddressLength() { return IPV6_ADDR_SIZE; }

private:
    void initData(std::string_view address){
        if (extractIPv6Address(address)){
            m_printable_address = address;
            m_valid = true;
        }
    }

    void initData(const uint8_t *p_ipv6_addr){
        if (extractPrintableAddress(p_ipv6_addr)){
            std::copy(p_ipv6_addr, p_ipv6_addr + IPV6_ADDR_SIZE, m_ipv6_address.begin());
            m_valid = true;
        }
    }

    void resetData(){
        m_ipv6_address.fill(0);
        m_printable_address.clear();
        m_valid = false;
    }

    bool extractIPv6Address(std::string_view address){
         void *p_addr = reinterpret_cast<void*>(&m_ipv6_address);
         return sys_call_noerr_eval(::inet_pton, PF_INET6, address.data(), p_addr);
    }

    bool extractPrintableAddress(const void * src){
         char dest[INET6_ADDRSTRLEN];
         memset(dest, 0, INET6_ADDRSTRLEN);
         m_printable_address.reserve(INET6_ADDRSTRLEN);
         if (nullptr != ::inet_ntop(PF_INET6, src, dest, INET6_ADDRSTRLEN)){
             m_printable_address.assign(dest, strnlen(dest, INET6_ADDRSTRLEN));
             return true;
         }
         return false;
    }

private:
    std::array<uint8_t, 16> m_ipv6_address;
    std::string m_printable_address {};
    bool m_valid {false};
};

using IPV4HostAddr = HostAddress<ipv4_domain>;

} //inet

} //infra

#endif // HOSTADDRESS_HPP
