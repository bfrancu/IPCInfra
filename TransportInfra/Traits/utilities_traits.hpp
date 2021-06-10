#ifndef UTILITIES_TRAITS_HPP
#define UTILITIES_TRAITS_HPP

#include "TransportDefinitions.h"
#include "traits_utils.hpp"
#include <type_traits>

namespace infra
{

template<typename Endpoint, 
         typename = traits::select_if_t<def::has_type_Device<Endpoint>, 
                                        std::true_type,
                                        std::false_type>>
struct HasConnectableDevice;

template<typename Endpoint>
struct HasConnectableDevice<Endpoint, std::false_type> : std::false_type
{};

template<typename Endpoint>
struct HasConnectableDevice<Endpoint, std::true_type>
{
    using Device = typename Endpoint::Device;
    static constexpr bool value = def::has_member_connect<Device>::value && def::has_member_disconnect<Device>::value;
};


template<typename Device,
         typename = traits::select_if_t<def::has_member_connect<Device>,
                                        std::true_type,
                                        std::false_type>>
struct IsConnectableDeviceT;

template<typename Device>
struct IsConnectableDeviceT<Device, std::false_type> : std::false_type
{};

template<typename Device>
struct IsConnectableDeviceT<Device, std::true_type> : def::has_member_disconnect<Device>
{};

template<typename Device>
struct IsSendableDeviceT : def::has_member_send<Device>
{};

template<typename Device>
struct IsWritableDeviceT : def::has_member_write<Device>
{};

template<typename Device>
using IsPassiveConnectableDevice = traits::apply_predicates<Device, def::has_member_bind,
                                                                    def::has_member_listen,
                                                                    def::has_member_accept>;
/*
template<typename Device, 
         typename = traits::select_if_t<def::has_member_bind<Device>,
                                       std::true_type,
                                       std::false_type>>
struct IsPassiveConnectableDevice; 

template<typename Device>
struct IsPassiveConnectableDevice<Device, std::false_type> : std::false_type
{};

template<typename Device>
struct IsPassiveConnectableDevice<Device, std::true_type>
{
    static constexpr bool value = def::has_member_listen<Device>::value ? def::has_member_accept<Device>::value : false;
};
*/


}//infra

#endif
