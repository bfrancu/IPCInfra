#ifndef DEVICEFACTORY_HPP
#define DEVICEFACTORY_HPP

#include "Host.hpp"
#include "utilities.hpp"
#include "Devices/EmptyDevice.hpp"
#include "Devices/Sockets/SocketDevice.hpp"
#include "Devices/Sockets/HostAddress.hpp"
#include "Devices/Sockets/InetSocketAddress.hpp"
#include "Devices/Sockets/UnixSocketAddress.h"
#include "Devices/Pipes/NamedPipeFactory.h"

namespace infra
{

using IPV4HostAddr = inet::HostAddress<ipv4_domain>;
using IPV6HostAddr = inet::HostAddress<ipv6_domain>;
using IPV4NetworkAddress = inet::NetworkAddress<IPV4HostAddr>;
using IPV6NetworkAddress = inet::NetworkAddress<IPV6HostAddr>;
using IPV4InetSocketAddress = inet::InetSocketAddress<IPV4NetworkAddress>;
using IPV6InetSocketAddress = inet::InetSocketAddress<IPV6NetworkAddress>;

enum EDeviceType
{
    E_UNDEFINED_DEVICE = 0,
    E_IPV4_TCP_SOCKET_DEVICE,
    E_IPV4_UDP_SOCKET_DEVICE,
    E_IPV6_TCP_SOCKET_DEVICE,
    E_IPV6_UDP_SOCKET_DEVICE,
    E_UNIX_STREAM_SOCKET_DEVICE,
    E_UNIX_DGRAM_SOCKET_DEVICE,
    E_READING_FIFO_DEVICE,
    E_WRITING_FIFO_DEVICE,
    E_LAST_DEVICE,
};

template <EDeviceType device_t>
struct Device2Type
{
    enum {value = utils::to_underlying(device_t)};
};

struct socket_device_t
{};

struct unix_socket_device_t
{};


template <typename domain, typename io_profile, typename = void>
class DeviceFactory2
{};

/*template<typename device_t>
class DeviceFactory<device_t, std::enable_if_t<std::is_same_v<, >>>
{};*/

template<EDeviceType tag>
class DeviceFactory
{};

template<>
class DeviceFactory<EDeviceType::E_IPV4_TCP_SOCKET_DEVICE>
{
public:
    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
        return Host<SocketDevice<ResourceHandler, IPV4InetSocketAddress, ipv4_domain, stream_socket>, Policies...>{};
    }
};

template<>
class DeviceFactory<EDeviceType::E_IPV4_UDP_SOCKET_DEVICE>
{
public:
    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
        return Host<SocketDevice<ResourceHandler, IPV4InetSocketAddress, ipv4_domain, datagram_socket>, Policies...>{};
    }
};

template<>
class DeviceFactory<EDeviceType::E_IPV6_TCP_SOCKET_DEVICE>
{
public:
    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
        return Host<SocketDevice<ResourceHandler, IPV6InetSocketAddress, ipv6_domain, stream_socket>, Policies...>{};
    }
};

template<>
class DeviceFactory<EDeviceType::E_IPV6_UDP_SOCKET_DEVICE>
{
public:
    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
        return Host<SocketDevice<ResourceHandler, IPV6InetSocketAddress, ipv6_domain, datagram_socket>, Policies...>{};
    }
};

template<>
class DeviceFactory<EDeviceType::E_UNIX_STREAM_SOCKET_DEVICE>
{
public:
    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
        return Host<SocketDevice<ResourceHandler, unx::UnixSocketAddress, unix_domain, stream_socket>, Policies...>{};
    }
};


template<>
class DeviceFactory<EDeviceType::E_UNIX_DGRAM_SOCKET_DEVICE>
{
public:
    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
        return Host<SocketDevice<ResourceHandler, unx::UnixSocketAddress, unix_domain, datagram_socket>, Policies...>{};
    }
};


template<>
class DeviceFactory<EDeviceType::E_READING_FIFO_DEVICE>
{
public:
    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
       return Host<ReadingNamedPipeDevice<UnixResourceHandler>, Policies...>();
    }
};


template<>
class DeviceFactory<EDeviceType::E_WRITING_FIFO_DEVICE>
{
public:
    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
        return Host<WritingNamedPipeDevice<UnixResourceHandler>, Policies...>();
    }
};


/*
class DeviceFactory1
{



public:

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
   static constexpr decltype (auto) createDevice(EDeviceType device_type){
        switch (device_type)
        {
        case E_IPV4_TCP_SOCKET_DEVICE : return createDevice<ResourceHandler, Policies...>(Device2Type<EDeviceType::E_IPV4_TCP_SOCKET_DEVICE>{});
            break;
        case E_IPV4_UDP_SOCKET_DEVICE : return createDevice<ResourceHandler, Policies...>(Device2Type<EDeviceType::E_IPV4_UDP_SOCKET_DEVICE>{});
            break;
        case E_IPV6_TCP_SOCKET_DEVICE : return createDevice<ResourceHandler, Policies...>(Device2Type<EDeviceType::E_IPV6_TCP_SOCKET_DEVICE>{});
            break;
        case E_IPV6_UDP_SOCKET_DEVICE : return createDevice<ResourceHandler, Policies...>(Device2Type<EDeviceType::E_IPV6_UDP_SOCKET_DEVICE>{});
            break;
        case E_UNIX_STREAM_SOCKET_DEVICE : return createDevice<ResourceHandler, Policies...>(Device2Type<EDeviceType::E_UNIX_STREAM_SOCKET_DEVICE>{});
            break;
        case E_UNIX_DGRAM_SOCKET_DEVICE : return createDevice<ResourceHandler, Policies...>(Device2Type<EDeviceType::E_UNIX_DGRAM_SOCKET_DEVICE>{});
            break;
        case E_READING_FIFO_DEVICE : return createDevice<ResourceHandler, Policies...>(Device2Type<EDeviceType::E_READING_FIFO_DEVICE>{});
            break;
        case E_WRITING_FIFO_DEVICE : return createDevice<ResourceHandler, Policies...>(Device2Type<EDeviceType::E_WRITING_FIFO_DEVICE>{});
            break;
        default: return Host<EmptyDevice<ResourceHandler>>{};

        }
    }

protected:
    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(Device2Type<EDeviceType::E_IPV4_TCP_SOCKET_DEVICE>){
        return Host<SocketDevice<ResourceHandler, IPV4InetSocketAddress, ipv4_domain, stream_socket>, Policies...>{};
    }

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(Device2Type<EDeviceType::E_IPV4_UDP_SOCKET_DEVICE>){
        return Host<SocketDevice<ResourceHandler, IPV4InetSocketAddress, ipv4_domain, datagram_socket>, Policies...>{};
    }

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(Device2Type<EDeviceType::E_IPV6_TCP_SOCKET_DEVICE>){
        return Host<SocketDevice<ResourceHandler, IPV6InetSocketAddress, ipv6_domain, stream_socket>, Policies...>{};
    }

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(Device2Type<EDeviceType::E_IPV6_UDP_SOCKET_DEVICE>){
        return Host<SocketDevice<ResourceHandler, IPV6InetSocketAddress, ipv6_domain, datagram_socket>, Policies...>{};
    }

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(Device2Type<EDeviceType::E_UNIX_STREAM_SOCKET_DEVICE>){
        return Host<SocketDevice<ResourceHandler, unx::UnixSocketAddress, unix_domain, stream_socket>, Policies...>{};
    }

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(Device2Type<EDeviceType::E_UNIX_DGRAM_SOCKET_DEVICE>){
        return Host<SocketDevice<ResourceHandler, unx::UnixSocketAddress, unix_domain, datagram_socket>, Policies...>{};
    }

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(Device2Type<EDeviceType::E_READING_FIFO_DEVICE>){
    }

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(Device2Type<EDeviceType::E_WRITING_FIFO_DEVICE>){

    }


protected:
    static inline std::variant<
        Device2Type<EDeviceType::E_UNDEFINED_DEVICE>,
        Device2Type<EDeviceType::E_IPV4_TCP_SOCKET_DEVICE>,
        Device2Type<EDeviceType::E_IPV4_UDP_SOCKET_DEVICE>,
        Device2Type<EDeviceType::E_IPV6_TCP_SOCKET_DEVICE>,
        Device2Type<EDeviceType::E_IPV6_UDP_SOCKET_DEVICE>,
        Device2Type<EDeviceType::E_UNIX_STREAM_SOCKET_DEVICE>,
        Device2Type<EDeviceType::E_UNIX_DGRAM_SOCKET_DEVICE>,
        Device2Type<EDeviceType::E_READING_FIFO_DEVICE>,
        Device2Type<EDeviceType::E_WRITING_FIFO_DEVICE>
        >device_types_variant {};

private:
    static inline std::once_flag once_flag{};

};
*/

} // infra

#endif // DEVICEFACTORY_HPP
