#include <sstream>

#include "InetUtilities.h"


namespace infra
{

/*
template<typename Domain>
bool operator==(const HostAddress<Domain> & first, const HostAddress<Domain> & second){
    return first.toStringReadOnly() == second.toStringReadOnly();
}

template<typename Domain>
bool operator!=(const HostAddress<Domain> & first, const HostAddress<Domain> & second){
    return !(first==second);
}

template<typename Domain>
bool operator<(const HostAddress<Domain> & first, const HostAddress<Domain> & second){
    return first.toStringReadOnly() < second.toStringReadOnly();
}

bool operator==(const HostAddress<ipv4_domain> & address, const in_addr & ipv4_addr){
    return address.getAddress() == ipv4_addr.s_addr;
}

bool operator==(const HostAddress<ipv6_domain> & address, const in6_addr & ipv6_addr){
      uint8_t addr_arr[HostAddress<ipv6_domain>::IPV6_ADDR_SIZE];
      if (address.getAddress(addr_arr)){
            return std::equal(addr_arr, addr_arr + HostAddress<ipv6_domain>::IPV6_ADDR_SIZE,
                              reinterpret_cast<const uint8_t*>(&ipv6_addr));
      }
      return false;
}*/

template<typename HostAddress>
bool operator==(const HostAddress & address, std::string_view strview_address){
    return address.toStringReadOnly() == strview_address;
}

std::string addressToString(const uint8_t *arr, size_t size){
    std::stringstream stream;
    for (size_t i = 0; i < size; ++i){
        stream << static_cast<int>(arr[i]);
        if (size -1 == i) break;
        stream << ".";
    }
    return stream.str();
}

} // infra
