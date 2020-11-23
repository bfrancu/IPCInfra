#ifndef DEVICE_TRAITS_HPP
#define DEVICE_TRAITS_HPP
#include "handler_traits.hpp"
#include "traits_utils.hpp"

namespace infra
{

DEFINE_HAS_TYPE(address_type);

struct sequential_device{};

struct read_only_profile{};
struct write_only_profile{};

template<typename T, typename = std::void_t<>>
struct HasSequentialType : std::false_type
{};

template<typename T>
struct HasSequentialType<T, std::void_t<typename std::remove_reference_t<T>::sequential>>
        : std::true_type
{};

template <typename Device, bool = HasSequentialType<Device>::value>
struct device_traits : public handler_traits<Device>{
     using sequential   = typename Device::sequential;
};

template <typename Device>
struct device_traits<Device, false> : public handler_traits<Device>
{};

template<typename T, typename = std::void_t<>>
struct HasSequentialTypeT : std::false_type
{};


//template<typename T>
//struct is_sequential<T, std::void_t<decltype(typename T::sequential())>> : std::true_type
//{};

template<typename T>
struct HasSequentialTypeT<T, std::void_t<decltype (std::declval<typename T::sequential>())>> : std::true_type
{};

template<typename T, bool = HasSequentialTypeT<T>::value>
struct SequentialTypeT{
    using Type = typename T::sequential;
};

template<typename T>
struct SequentialTypeT<T, false>
{};

template<typename T, typename = std::void_t<>>
struct IsSeekableDeviceT : std::false_type
{};

/*
template<typename T>
struct IsSeekableDeviceT<T, std::void_t<std::enable_if_t<std::is_same_v<typename SequentialTypeT<T>::Type, sequential_device>>>> : std::true_type
{};
*/

template<typename T>
struct IsSeekableDeviceT<T, std::void_t<std::enable_if_t<std::is_same_v<typename T::sequential, sequential_device >>>> : std::true_type
{};

template<typename, typename = std::void_t<>>
struct HasMemberT_SetLatestError
        : std::false_type {};

template<typename T>
struct HasMemberT_SetLatestError<T, std::void_t<decltype (&T::setLatestError)>>
        : std::true_type {};
} //infra


template<typename T, typename = std::void_t<>>
struct IsFifoDeviceT : std::false_type
{};


template<typename T>
struct IsFifoDeviceT<T, std::void_t<typename T::io_profile>> : std::true_type
{};


#endif // DEVICE_TRAITS_HPP
