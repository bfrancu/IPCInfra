#ifndef FIFO_TRAITS_HPP
#define FIFO_TRAITS_HPP
#include "device_traits.hpp"
#include "Devices/Pipes/NamedPipeDevice.hpp"
#include "Devices/Pipes/NamedPipeAddress.h"

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
    using address_type      = std::conditional<HasTypeT_address_type<Device>::value,
                                               typename Device::address_type,
                                               NamedPipeAddress>;
};


template<typename Device, typename = std::void_t<>>
struct IsNamedPipeDeviceT : std::false_type 
{};

template<typename Device>
struct IsNamedPipeDeviceT<Device,
                          std::enable_if_t<std::conjunction_v<HasTypeT_io_profile<Device>,
                                                            HasTypeT_address_type<Device>>>> : std::true_type
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
