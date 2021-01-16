#ifndef DEVICEFACTORY_HPP
#define DEVICEFACTORY_HPP

#include "Host.hpp"
#include "DeviceDefinitions.h"
#include "utilities.hpp"
#include "Devices/EmptyDevice.hpp"
#include "Devices/Sockets/SocketDevice.hpp"
#include "Devices/Pipes/NamedPipeFactory.h"

namespace infra
{

/*
template <EDeviceType device_t>
struct Device2Type
{
    enum {value = utils::to_underlying(device_t)};
};*/

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

template<std::size_t tag>
struct Device2Type
{
    template<typename ResourceHandler>
    using device_type = EmptyDevice<ResourceHandler>;
};

template<>
struct Device2Type<static_cast<std::size_t>(EDeviceType::E_IPV4_TCP_SOCKET_DEVICE)>
{
    template<typename ResourceHandler>
    using device_type = SocketDevice<ResourceHandler, IPV4InetSocketAddress, ipv4_domain, stream_socket>;
};

template<>
struct Device2Type<static_cast<std::size_t>(EDeviceType::E_IPV4_UDP_SOCKET_DEVICE)>
{
    template<typename ResourceHandler>
    using device_type = SocketDevice<ResourceHandler, IPV4InetSocketAddress, ipv4_domain, datagram_socket>;
};

template<>
struct Device2Type<static_cast<std::size_t>(EDeviceType::E_IPV6_TCP_SOCKET_DEVICE)>
{
    template<typename ResourceHandler>
    using device_type = SocketDevice<ResourceHandler, IPV6InetSocketAddress, ipv6_domain, stream_socket>;
};

template<>
struct Device2Type<static_cast<std::size_t>(EDeviceType::E_IPV6_UDP_SOCKET_DEVICE)>
{
    template<typename ResourceHandler>
    using device_type = SocketDevice<ResourceHandler, IPV6InetSocketAddress, ipv6_domain, datagram_socket>;
};

template<>
struct Device2Type<static_cast<std::size_t>(EDeviceType::E_UNIX_STREAM_SOCKET_DEVICE)>
{
    template<typename ResourceHandler>
    using device_type = SocketDevice<ResourceHandler, unx::UnixSocketAddress, unix_domain, stream_socket>;
};

template<>
struct Device2Type<static_cast<std::size_t>(EDeviceType::E_UNIX_DGRAM_SOCKET_DEVICE)>
{
    template<typename ResourceHandler>
    using device_type = SocketDevice<ResourceHandler, unx::UnixSocketAddress, unix_domain, datagram_socket>;
};

template<>
struct Device2Type<static_cast<std::size_t>(EDeviceType::E_READING_FIFO_DEVICE)>
{
    template<typename ResourceHandler>
    using device_type = ReadingNamedPipeDevice<ResourceHandler>;
};

template<>
struct Device2Type<static_cast<std::size_t>(EDeviceType::E_WRITING_FIFO_DEVICE)>
{
    template<typename ResourceHandler>
    using device_type = WritingNamedPipeDevice<ResourceHandler>;
};

template<std::size_t tag, typename Enable = void>
class DeviceFactory
{
public:
    template<typename ResourceHandler>
    using device_type = typename Device2Type<tag>::template device_type<ResourceHandler>;

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
        return Host<device_type<ResourceHandler>, Policies...>{};
    }

    template <typename ResourceHandler, typename PolciesList>
    static constexpr decltype (auto) createDevice(){
        return PackHostT<device_type<ResourceHandler>, PolciesList>{};
    }

};

/*
template<>
class DeviceFactory<static_cast<std::size_t>(EDeviceType::E_IPV4_TCP_SOCKET_DEVICE)>
{
public:
    template<typename ResourceHandler>
    using device_type = SocketDevice<ResourceHandler, IPV4InetSocketAddress, ipv4_domain, stream_socket>;

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
        return Host<device_type<ResourceHandler>, Policies...>{};
    }
};

template<>
class DeviceFactory<static_cast<std::size_t>(EDeviceType::E_IPV4_UDP_SOCKET_DEVICE)>
{
public:
    template<typename ResourceHandler>
    using device_type = SocketDevice<ResourceHandler, IPV4InetSocketAddress, ipv4_domain, datagram_socket>;

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
        return Host<device_type<ResourceHandler>, Policies...>{};
    }
};

template<>
class DeviceFactory<static_cast<std::size_t>(EDeviceType::E_IPV6_TCP_SOCKET_DEVICE)>
{
public:
    template<typename ResourceHandler>
    using device_type = SocketDevice<ResourceHandler, IPV6InetSocketAddress, ipv6_domain, stream_socket>;

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
        return Host<device_type<ResourceHandler>, Policies...>{};
    }
};

template<>
class DeviceFactory<static_cast<std::size_t>(EDeviceType::E_IPV6_UDP_SOCKET_DEVICE)>
{
public:
    template<typename ResourceHandler>
    using device_type = SocketDevice<ResourceHandler, IPV6InetSocketAddress, ipv6_domain, datagram_socket>;

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
        return Host<device_type<ResourceHandler>, Policies...>{};
    }
};

template<>
class DeviceFactory<static_cast<std::size_t>(EDeviceType::E_UNIX_STREAM_SOCKET_DEVICE)>
{
public:
    template<typename ResourceHandler>
    using device_type = SocketDevice<ResourceHandler, unx::UnixSocketAddress, unix_domain, stream_socket>;

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
        return Host<device_type<ResourceHandler>, Policies...>{};
    }
};


template<>
class DeviceFactory<static_cast<std::size_t>(EDeviceType::E_UNIX_DGRAM_SOCKET_DEVICE)>
{
public:
    template<typename ResourceHandler>
    using device_type = SocketDevice<ResourceHandler, unx::UnixSocketAddress, unix_domain, datagram_socket>;
    
    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
        return Host<device_type<ResourceHandler>, Policies...>{};
    }
};


template<>
class DeviceFactory<static_cast<std::size_t>(EDeviceType::E_READING_FIFO_DEVICE)>
{
public:
    template<typename ResourceHandler>
    using device_type = ReadingNamedPipeDevice<ResourceHandler>;

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
       return Host<device_type<ResourceHandler>, Policies...>();
    }
};


template<>
class DeviceFactory<static_cast<std::size_t>(EDeviceType::E_WRITING_FIFO_DEVICE)>
{
public:
    template<typename ResourceHandler>
    using device_type = WritingNamedPipeDevice<ResourceHandler>;

    template <typename ResourceHandler,
             template<typename...> typename... Policies>
    static constexpr decltype (auto) createDevice(){
        return Host<device_type<ResourceHandler>, Policies...>();
    }
};
*/

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
