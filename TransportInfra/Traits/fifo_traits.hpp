#ifndef FIFO_TRAITS_HPP
#define FIFO_TRAITS_HPP
#include "device_traits.hpp"
#include "Devices/Pipes/NamedPipeDevice.hpp"
#include "Devices/Pipes/NamedPipeAddress.h"
#include "traits_utils.hpp"
#include <type_traits>

namespace infra
{

/*
template<typename T, typename = std::void_t<>>
struct HasAddressTypeT : std::false_type
{};

template<typename T>
struct HasAddressTypeT<T, std::void_t<typename T::address_type>> : std::true_type
{};*/

DEFINE_HAS_TYPE(io_profile);

template <typename Device>
struct fifo_traits : public device_traits<Device>
{
    using io_profile        = typename Device::io_profile;
    using address_type      = std::conditional_t<has_type_address_type<Device>::value,
                                               typename Device::address_type,
                                               NamedPipeAddress>;
};


template<typename Device, typename = std::void_t<>>
struct IsNamedPipeDeviceT : std::false_type 
{};

template<typename Device>
struct IsNamedPipeDeviceT<Device,
                          std::enable_if_t<std::conjunction_v<has_type_io_profile<Device>
                                                             , has_type_address_type<Device>
                                                              >>> : std::true_type
{};
                                                            
template<typename Device,
         typename IsNamedPipe = traits::select_if_t<IsNamedPipeDeviceT<Device>,
                                                    std::true_type,
                                                    std::false_type>>
struct UnixNamedPipeDevice;

template<typename Device>
struct UnixNamedPipeDevice<Device, std::false_type> : std::false_type
{};

template<typename Device>
struct UnixNamedPipeDevice<Device, std::true_type> : HasUnixHandleTypeT<Device>
{};

template<typename T, typename = std::void_t<>>
struct IsFifoDeviceT : std::false_type
{};


template<typename T>
struct IsFifoDeviceT<T, std::void_t<typename T::io_profile>> : std::true_type
{};

/*
template<typename Device>
struct IsNamedPipeDeviceT<Device, std::void_t<decltype(std::declval<fifo_traits<Device>>())>>
                        : std::true_type
{};
*/

/*
template<template <typename > typename Device>
struct IsNamedPipeDeviceT<Device<void>, std::enable_if_t<std::is_base_of_v<NamedPipeDevice<void>, 
                                                                           Device<void>>>>
                         : std::true_type
{};
*/
} //infra

#endif
