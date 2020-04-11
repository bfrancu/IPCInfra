#ifndef HANDLER_TRAITS_HPP
#define HANDLER_TRAITS_HPP
#include <type_traits>


namespace infra
{

struct unix_platform{};

struct my_custom_platform{};

struct sequential_device{};

template <typename T, typename = std::void_t<>>
struct handler_traits
{
    using handle_type = typename T::handle_type;
    using platform = typename T::platform;

    template<typename U>
    static constexpr std::enable_if_t<std::is_default_constructible_v<std::remove_reference_t<U>>,handle_type>
    defaultValue(){return U();}
};


template <typename T>
constexpr bool HasUnixHandle
= std::is_same<unix_platform,
               typename handler_traits<T>::platform
               >::value;

template <typename T>
using UnixHandlerT = std::enable_if_t<HasUnixHandle<T>>;

template <typename T>
constexpr bool UnixCompatibleHandle = std::is_same<int, std::decay_t<T>>::value;


template <typename T>
using UnixCompatibleHandleT = std::enable_if_t<UnixCompatibleHandle<T>>;


class UnixResourceHandler;

template <typename T>
struct handler_traits<T, std::void_t<std::enable_if_t<std::is_same_v<unix_platform,
                                                                     typename T::platform>>>>
{
    using handle_type = typename T::handle_type;
    using platform = typename T::platform;
    static constexpr handle_type defaultValue(){
        return -1;
    }
};

/*
template<typename T, typename = std::void_t<>>
struct HasHandleTypeT : std::false_type
{};

template<typename T>
struct HasHandleTypeT<T, std::void_t<typename std::remove_reference_t<T>::handle_type>> : std::true_type
{};

template<typename T, bool = HasHandleTypeT<T>::value>
struct HandleTypeT{
    using Type = typename T::handle_type;
};

template<typename T>
struct HandleTypeT<T, false>
{};
*/

template<typename T,
         typename = std::void_t<>>
struct HasUnixHandleTypeT : std::false_type
{};

/*
template<typename T>
struct HasUnixHandleTypeT<T, std::void_t<std::enable_if_t<std::is_same_v<typename HandleTypeT<T>::Type, int>>>>
        : std::true_type
{};
*/

template<typename T>
struct HasUnixHandleTypeT<T, std::void_t<std::enable_if_t<std::is_same_v<typename T::handle_type, int>>>>
        : std::true_type
{};



} //infra
#endif // HANDLER_TRAITS_HPP
