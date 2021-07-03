#ifndef NON_TYPELIST_HPP
#define NON_TYPELIST_HPP
#include <tuple>
#include "default_traits.hpp"

namespace infra
{

namespace meta
{
namespace ntl
{

template<auto... elements>
struct non_typelist {};

template<typename NonTList>
struct is_empty
{
    static constexpr bool value = false;
};

template<>
struct is_empty<non_typelist<>>
{
    static constexpr bool value = true;
};

template<typename NonTList>
struct member_type;

template<>
struct member_type<non_typelist<>>
{
    using type = int;
};

template<typename T, T... Es>
struct member_type<non_typelist<Es...>>
{
    using type = T;
};

template<typename NonTList>
using member_type_t = typename member_type<NonTList>::type;

template<typename NonTList>
constexpr bool is_empty_v = is_empty<NonTList>::value;

template<typename NonTList>
struct size;

template<auto... Es>
struct size<non_typelist<Es...>>
{
    static constexpr std::size_t value = sizeof...(Es);
};

template<typename NonTList>
constexpr std::size_t size_v = size<NonTList>::value;

template<typename NonTList>
struct front;

template<typename T, T Head, T... Tails>
struct front<non_typelist<Head, Tails...>>
{
    static constexpr T value = Head;
};

template<>
struct front<non_typelist<>>
{
    static constexpr auto value = traits::default_value<int>::value;
};

template<typename NonTList>
constexpr auto front_v = front<NonTList>::value;

template<typename NonTList>
struct back;

template<typename T, T Head, T... Tails>
struct back<non_typelist<Head, Tails...>>
{
    static constexpr auto value = back<non_typelist<Tails...>>::value;
};

template<auto Tail>
struct back<non_typelist<Tail>>
{
    static constexpr auto value = Tail;
};

template<typename NonTList>
constexpr auto back_v = back<NonTList>::value;

template<typename NonTList>
struct pop_front;

template<typename T, T Head, T... Tails>
struct pop_front<non_typelist<Head, Tails...>>
{
    using type = non_typelist<Tails...>;
};

template<typename NonTList>
using pop_front_t = typename pop_front<NonTList>::type;

template<typename NonTList, auto NewElement>
struct push_front;

template<typename T, T NewElement, T... Tails>
struct push_front<non_typelist<Tails...>, NewElement>
{
    using type = non_typelist<NewElement, Tails...>;
};

template<typename NonTList, auto NewElement>
using push_front_t = typename push_front<NonTList, NewElement>::type;

template<typename NonTList, auto NewElement>
struct push_back;

template<typename T, T NewElement, T... Tails>
struct push_back<non_typelist<Tails...>, NewElement>
{
    using type = non_typelist<Tails..., NewElement>;
};

template<typename NonTList, auto NewElement>
using push_back_t = typename push_back<NonTList, NewElement>::type;

template<typename NonTList, std::size_t N, typename T = member_type_t<NonTList>, typename = void>
struct nth_element;

template<typename T, T... Es, std::size_t N>
struct nth_element<non_typelist<Es...>, N, T, std::enable_if_t<(N>0)>> : nth_element<pop_front_t<non_typelist<Es...>>, N-1, T>
{};

template<typename NonTList, typename T>
struct nth_element<NonTList, 0, T> : front<NonTList>
{};

template<std::size_t N, typename T>
struct nth_element<non_typelist<>, N, T, std::enable_if_t<(N>0)>>
{
    static constexpr T value = traits::default_value<T>::value;
};

template<typename NonTList, std::size_t N>
constexpr auto nth_element_v = nth_element<NonTList, N>::value;

}//ntl
}//meta
}//infra

#endif // NON_TYPELIST_HPP
