#ifndef DEVICE_CONSTRAINTS_HPP
#define DEVICE_CONSTRAINTS_HPP
#include "socket_traits.hpp"
#include "fifo_traits.hpp"


namespace infra
{
namespace traits
{

template<typename Device, 
         typename = std::enable_if_t<HasUnixHandleTypeT<Device>::value>>
using UnixDevice = Device;

template<typename Device,
         typename = std::enable_if_t<IsSocketDeviceT<Device>::value>>
using SocketDevice = Device;

template<typename Device,
         typename = std::enable_if_t<IsStreamSocketDeviceT<Device>::value>>
using StreamSocketDevice = Device;

template<typename Device,
         typename = std::enable_if_t<IsDatagramSocketDeviceT<Device>::value
                                     && std::negation_v<IsStreamSocketDeviceT<Device>> 
                                     && std::negation_v<IsNamedPipeDeviceT<Device>> 
                                     >>
using DatagramSocketDevice = Device;

template<typename Device,
         typename = std::enable_if_t<IsUnixSocketDeviceT<Device>::value
                                     //&& std::negation_v<IsNamedPipeDeviceT<Device>>
                            >>
using UnixSocketDevice = Device;

template<typename Device,
         typename = std::enable_if_t<IsNamedPipeDeviceT<Device>::value
                                     && std::negation_v<IsSocketDeviceT<Device>> 
                                     >>
using NamedPipeDevice = Device;

template<typename Device,
         typename = std::enable_if_t<UnixNamedPipeDevice<Device>::value>>
using UnixNamedPipeDevice = Device;

template<typename Device,
         typename = std::enable_if_t<std::conjunction_v<HasUnixHandleTypeT<Device>,
                                                        IsFifoDeviceT<Device>>>>
using FifoDevice = Device;

template<typename Device, typename = void>
struct select_traits : device_traits<Device>
{};

template<typename Device>
struct select_traits<NamedPipeDevice<Device>> : fifo_traits<Device>
{};

template<typename Device>
struct select_traits<Device, std::enable_if_t<IsSocketDeviceT<Device>::value>> : socket_traits<Device>
{};

}//traits
}//infra

#endif
