#ifndef DEVICE_DEFINITIONS_H
#define DEVICE_DEFINITIONS_H

#include "Devices/Sockets/HostAddress.hpp"
#include "Devices/Sockets/InetSocketAddress.hpp"
#include "Devices/Sockets/UnixSocketAddress.h"

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
    E_UNDEFINED_DEVICE = -1,
    E_IPV4_TCP_SOCKET_DEVICE,
    E_IPV4_UDP_SOCKET_DEVICE,
    E_IPV6_TCP_SOCKET_DEVICE,
    E_IPV6_UDP_SOCKET_DEVICE,
    E_UNIX_STREAM_SOCKET_DEVICE,
    E_UNIX_DGRAM_SOCKET_DEVICE,
    E_READING_FIFO_DEVICE,
    E_WRITING_FIFO_DEVICE,
    E_LAST_DEVICE
};

static constexpr auto ipv4_strm_tag = static_cast<std::size_t>(EDeviceType::E_IPV4_TCP_SOCKET_DEVICE);
static constexpr auto ipv4_dgram_tag = static_cast<std::size_t>(EDeviceType::E_IPV4_UDP_SOCKET_DEVICE);
static constexpr auto ipv6_strm_tag = static_cast<std::size_t>(EDeviceType::E_IPV6_TCP_SOCKET_DEVICE);
static constexpr auto ipv6_dgram_tag = static_cast<std::size_t>(EDeviceType::E_IPV6_UDP_SOCKET_DEVICE);
static constexpr auto unx_strm_tag = static_cast<std::size_t>(EDeviceType::E_UNIX_STREAM_SOCKET_DEVICE);
static constexpr auto unx_dgram_tag = static_cast<std::size_t>(EDeviceType::E_UNIX_STREAM_SOCKET_DEVICE);
static constexpr auto read_fifo_tag = static_cast<std::size_t>(EDeviceType::E_READING_FIFO_DEVICE);
static constexpr auto write_fifo_tag = static_cast<std::size_t>(EDeviceType::E_WRITING_FIFO_DEVICE);

}

#endif
