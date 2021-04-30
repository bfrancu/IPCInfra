#ifndef SOCKET_TRAITS_HPP
#define SOCKET_TRAITS_HPP
#include <sys/un.h>
#include <netinet/ip.h>

#include <type_traits>
#include <utility>

#include "device_traits.hpp"

namespace infra
{

struct unix_domain{};
//struct inet_domain{};
struct ipv4_domain{};
struct ipv6_domain{};

struct stream_socket{};
struct datagram_socket{};

template <typename Device>
struct socket_traits : public device_traits<Device>
{
  using socket_domain = typename Device::socket_domain;
  using socket_type   = typename Device::socket_type;
  using address_type  = typename Device::address_type;
};

DEFINE_HAS_TYPE(socket_domain);
DEFINE_HAS_TYPE(socket_type);

template <typename Device, typename = std::void_t<>>
struct IsSocketDeviceT : std::false_type
{};

/*
template <typename Device>
struct IsSocketDeviceT<Device,
                       std::void_t<decltype(std::declval<socket_traits<Device>>())>> : std::true_type
{};
*/

template <typename Device>
struct IsSocketDeviceT<Device,
                       std::enable_if_t<std::conjunction_v<has_type_socket_domain<Device>, 
                                                         has_type_socket_type<Device>,
                                                         has_type_address_type<Device>>>> 
                       : std::true_type
{};


/*
template<typename Device, typename = void>
struct IsStreamSocketDeviceT : std::false_type
{};

template<typename Device>
struct IsStreamSocketDeviceT<Device,
                       std::enable_if_t<std::conjunction<IsSocketDeviceT<Device>,
                                                         std::is_same<stream_socket, typename socket_traits<Device>::socket_type>>::value>>
        : std::true_type
{};
*/

template<typename Device>
using HasStreamSocketType = std::is_same<stream_socket, typename socket_traits<Device>::socket_type>;

template<typename Device>
using HasDatagramStreamSocketType = std::is_same<datagram_socket, typename socket_traits<Device>::socket_type>;

template<typename Device, 
         typename IsSocket = traits::select_if_t<IsSocketDeviceT<Device>, 
                                                 std::true_type,
                                                 std::false_type>>
struct IsStreamSocketDeviceT;

template<typename Device>
struct IsStreamSocketDeviceT<Device, std::false_type> : std::false_type
{};

template<typename Device>
struct IsStreamSocketDeviceT<Device, std::true_type> : HasStreamSocketType<Device>
{};


template<typename Device, 
         typename IsSocket = traits::select_if_t<IsSocketDeviceT<Device>, 
                                                                 std::true_type,
                                                                 std::false_type>>
struct IsDatagramSocketDeviceT;

template<typename Device>
struct IsDatagramSocketDeviceT<Device, std::false_type> : std::false_type
{};

template<typename Device>
struct IsDatagramSocketDeviceT<Device, std::true_type> : HasDatagramStreamSocketType<Device>
{};

template <typename ResourceHandler,
          typename SocketAddress>
struct UnixSocketRequires : std::conjunction<HasUnixHandleTypeT<ResourceHandler>,
                                             std::is_same<typename SocketAddress::address_type, sockaddr_un>>
{};

template <typename ResourceHandler,
          typename SocketAddress>
struct IPV4SocketRequires : std::conjunction<HasUnixHandleTypeT<ResourceHandler>,
                                             std::is_same<typename SocketAddress::address_type, sockaddr_in>>
{};

template <typename ResourceHandler,
          typename SocketAddress>
struct IPV6SocketRequires : std::conjunction<HasUnixHandleTypeT<ResourceHandler>,
                                             std::is_same<typename SocketAddress::address_type, sockaddr_in6>>
{};

template<typename NetworkAddress>
struct IsIPV4NetworkAddress : std::conjunction<std::is_same<ipv4_domain, typename NetworkAddress::inet_domain>>/*,
                                               std::negation<std::is_same<ipv6_domain, typename NetworkAddress::inet_domain>>>*/
{};

template<typename NetworkAddress>
struct IsIPV6NetworkAddress : std::conjunction<std::is_same<ipv6_domain, typename NetworkAddress::inet_domain>>/*,
                                               std::negation<std::is_same<ipv4_domain, typename NetworkAddress::inet_domain>>>*/
{};

template<typename SocketA, typename SocketB, typename = std::void_t<>>
struct CompatibleSocketDevices : std::false_type
{};

template<typename SocketA, typename SocketB>
struct CompatibleSocketDevices<SocketA,
                               SocketB,
                               std::enable_if_t<std::conjunction_v<std::is_same<typename SocketA::handle_type, typename SocketB::handle_type>,
                                                                   std::is_same<typename SocketA::platform, typename SocketB::platform>,
                                                                   std::is_same<typename SocketA::socket_domain, typename SocketB::socket_domain>,
                                                                   std::is_same<typename SocketA::socket_type, typename SocketB::socket_type>,
                                                                   std::is_convertible<typename SocketB::address_type, typename SocketA::address_type>
                                                         >>> : std::true_type
{};

/*template <typename T>
struct IsUnixPlatformSocketDeviceT : std::conjunction<HasUnixHandleTypeT<T>, IsSocketDeviceT<T>>
{};*/

template<typename Device,
         typename IsSocket =  traits::select_if_t<IsSocketDeviceT<Device>, 
                                                  std::true_type,
                                                  std::false_type>>
struct IsUnixPlatformSocketDeviceT;

template<typename Device>
struct IsUnixPlatformSocketDeviceT<Device, std::false_type> : std::false_type
{};

template<typename Device>
struct IsUnixPlatformSocketDeviceT<Device, std::true_type> : HasUnixHandleTypeT<Device>
{};

template<typename Device,
         typename IsUnixPlatformSocket = traits::select_if_t<IsUnixPlatformSocketDeviceT<Device>,
                                                             std::true_type,
                                                             std::false_type>>
struct IsUnixSocketDeviceT;

template<typename Device>
struct IsUnixSocketDeviceT<Device, std::false_type> : std::false_type
{};

template<typename Device>
struct IsUnixSocketDeviceT<Device, std::true_type>
{
    using SocketAddress = typename Device::address_type;
    static constexpr bool value = std::is_same_v<typename SocketAddress::address_type, sockaddr_un>;
};

} //infra
/*

  template<typename ResourceHandler,
           typename SockDomain,
           typename SockType
           typename = std::void_t<>>
  class Socket{};


  tempalate<UnixHandler, stream_socket, ipv4_domain>
  class Socket

{
    handle = ::socket(AF_INET, SOCK_STREAM);
}



*/

#endif // SOCKET_TRAITS_HPP
