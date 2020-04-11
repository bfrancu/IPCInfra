#ifndef INETUTILITIES_H
#define INETUTILITIES_H
#include <iosfwd>
#include <stdint.h>
#include <string_view>


namespace infra
{
namespace utils
{

namespace inet
{
std::string addressToString(const uint8_t *arr, size_t size);

/*
bool operator==(const HostAddress<ipv4_domain> & address, const in_addr & ipv4_addr);

bool operator==(const HostAddress<ipv6_domain> & address, const in6_addr & ipv6_addr);

template<typename Domain>
bool operator==(const HostAddress<Domain> & first, const HostAddress<Domain> & second);

template<typename Domain>
bool operator!=(const HostAddress<Domain> & first, const HostAddress<Domain> & second);

template<typename Domain>
bool operator<(const HostAddress<Domain> & first, const HostAddress<Domain> & second);

template<typename Domain>
bool operator==(const HostAddress<Domain> & address, std::string_view strview_address);
*/

} //inet
} //utils
} //infra
#endif // INETUTILITIES_H
