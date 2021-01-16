#ifndef TEMPLATE_TYPE_LIST_HPP
#define TEMPLATE_TYPE_LIST_HPP
#include <c++/10/bits/c++config.h>
#include <tuple>
#include <type_traits>

namespace infra
{
namespace meta
{
namespace ttl
{

template<typename...>
struct empty_tt{};

template<template<typename... > typename... Ts>
struct template_typelist
{};

template<template <typename...> typename Element>
struct pack
{};

template<typename TList>
struct is_empty
{
    static constexpr bool value = false;
};

template<>
struct is_empty<template_typelist<>> 
{
    static constexpr bool value = true;
};

template<typename TList>
constexpr bool is_empty_v = is_empty<TList>::value;

template<typename TList>
struct size;

template<template <typename...> typename... Ts>
struct size<template_typelist<Ts...>>
{
    static constexpr std::size_t value = sizeof...(Ts);
};

template<typename TList>
constexpr std::size_t size_v = size<TList>::value;

template<typename TList>
struct front;

template<template <typename...> typename Head, template <typename...> typename... Tails>
struct front<template_typelist<Head, Tails...>>
{
    template<typename... Ts>
    using type = Head<Ts...>;

    using packed_type = pack<Head>;
};

template<typename TList, typename... Ts>
using front_tt = typename front<TList>::template type<Ts...>;

template<typename TList>
struct pop_front;

template<template <typename...> typename Head, template <typename...> typename... Tails>
struct pop_front<template_typelist<Head, Tails...>>
{
    using type = template_typelist<Tails...>;
};

template<typename TList>
using pop_front_t = typename pop_front<TList>::type;

template<typename TList,template <typename...> typename NewElement>
struct push_front;

template<template <typename...> typename NewElement, template <typename...> typename... Tails>
struct push_front<template_typelist<Tails...>, NewElement>
{
    using type = template_typelist<NewElement, Tails...>;
};

template<typename TList, template <typename...> typename NewElement>
using push_front_t = typename push_front<TList, NewElement>::type;

template<typename TList,template <typename...> typename NewElement>
struct push_back;

template<template <typename...> typename NewElement, template <typename...> typename... Tails>
struct push_back<template_typelist<Tails...>, NewElement>
{
    using type = template_typelist<Tails..., NewElement>;
};

template<typename TList, template <typename...> typename NewElement>
using push_back_t = typename push_back<TList, NewElement>::type;

template<typename TList, std::size_t N>
struct nth_element : nth_element<pop_front_t<TList>, N-1>
{};

template<typename TList>
struct nth_element<TList, 0> : front<TList>
{};

template<std::size_t N>
struct nth_element<template_typelist<>, N> : front<template_typelist<empty_tt>>
{};

template<typename TList, std::size_t N, typename... Ts>
using nth_element_tt = typename nth_element<TList, N>::template type<Ts...>;

template<typename TList, template <typename...> typename Element>
struct erase_rec;

template<template <typename...> typename Element,
         template <typename...> typename Head,
         template <typename...> typename... Tails>
struct erase_rec<template_typelist<Head, Tails...>, Element>
{
    using tails = typename erase_rec<template_typelist<Tails...>, Element>::type;
    using type = push_front_t<tails, Head>;
};

template<template <typename...> typename Element,
         template <typename...> typename... Tails>
struct erase_rec<template_typelist<Element, Tails...>, Element>
{
    using type = typename erase_rec<template_typelist<Tails...>, Element>::type;
};

template<template <typename...> typename Element>
struct erase_rec<template_typelist<>, Element>
{
    using type = template_typelist<>;
};

template<typename TList, template <typename...> typename Element>
using erase_rec_t = typename erase_rec<TList, Element>::type;

template<typename TList>
struct to_tuple;

template<template <typename...> typename... Ts>
struct to_tuple<template_typelist<Ts...>>
{
    using type = std::tuple<pack<Ts>...>;
};

template<typename TList>
using to_tuple_t = typename to_tuple<TList>::type;

template<typename Tuple>
struct to_ttlist;

template<template <typename...> typename... Ts>
struct to_ttlist<std::tuple<pack<Ts>...>>
{
    using type = template_typelist<Ts...>;
};

template<typename Tuple>
using to_ttlist_t = typename to_ttlist<Tuple>::type;

template<typename... TLists>
struct concat
{
    using concat_tuples = decltype(std::tuple_cat(std::declval<to_tuple_t<TLists>>()...));
    using type = to_ttlist_t<concat_tuples>;
};

template<typename... TLists>
using concat_t = typename concat<TLists...>::type;

template<bool>
struct zero_or_one
{
    template<typename Element> struct helper;

    template<template <typename...> typename Element>
    struct helper<pack<Element>>
    {
        using type = template_typelist<>;
    };
};

template<>
struct zero_or_one<true>
{
    template<typename Element> struct helper;

    template<template <typename...> typename Element>
    struct helper<pack<Element>>
    {
        using type = template_typelist<Element>;
    };
};

template< template<template <typename...> typename > typename Pred, typename TList, bool FilterIn = true>
struct filter;

template<bool FilterIn, template<template <typename...> typename > typename Pred,
         template <typename...> typename... Ts>
struct filter<Pred, template_typelist<Ts...>, FilterIn>
{
    using type = concat_t<typename zero_or_one<(FilterIn ? Pred<Ts>::value 
                                                         : !Pred<Ts>::value)>::template helper<pack<Ts>>::type... >;
};

template< template<template <typename...> typename > typename Pred, typename TList, bool FilterIn = true>
using filter_t = typename filter<Pred, TList, FilterIn>::type;

template<template<template <typename...> typename > typename Pred, typename TList, std::size_t N>
struct find_first_by_index_helper;

template<std::size_t N,
         template<template <typename...> typename > typename Pred,
         template<typename...> typename Head,
         template <typename...> typename... Tails>
struct find_first_by_index_helper<Pred, template_typelist<Head, Tails...>, N>
{
    template<std::size_t Index, bool>
    struct helper
    {
        static constexpr int value = Index;
    };

    template<std::size_t Index>
    struct helper<Index, false>
    {
       static constexpr int value = find_first_by_index_helper<Pred, template_typelist<Tails...>, Index+1>::value; 
    };

    static constexpr int value = helper<N, Pred<Head>::value>::value;
};

template<std::size_t N, template<template <typename...> typename > typename Pred>
struct find_first_by_index_helper<Pred, template_typelist<>, N>
{
    static constexpr int value = -1;
};

template<template<template <typename...> typename > typename Pred, typename TList>
struct find_first_by_index
{
    static constexpr int value = find_first_by_index_helper<Pred, TList, 0>::value;
};

/*template<template<template <typename...> typename > typename Pred,
         template <typename...> typename... Ts>
struct find_first_by_index<Pred, template_typelist<Ts...>>
{
    static constexpr std::size_t index{0};

};*/
        
template<template<template <typename...> typename > typename Pred, typename TList>
constexpr bool find_first_by_index_v = find_first_by_index<Pred, TList>::value;

template<typename TList, template <typename...> typename Element>
struct erase;

template<template <typename...> typename Element, template <typename...> typename... Ts>
struct erase<template_typelist<Ts...>, Element>
{
    template<template<typename...> typename E>
    struct pred : std::is_same<pack<Element>, pack<E>>
    {};

    using type = filter_t<pred, template_typelist<Ts...>, false>;
};

template<typename TList, template <typename...> typename Element>
using erase_t = typename erase<TList, Element>::type;

template<typename TList>
struct remove_duplicates;

template<template <typename...> typename Head,
         template <typename...> typename... Tails>
struct remove_duplicates<template_typelist<Head, Tails...>>
{
    using tails_filtered_by_head = erase_t<template_typelist<Tails...>, Head>;
    using tails_duplicates_removed = typename remove_duplicates<tails_filtered_by_head>::type;
    using type = push_front_t<tails_duplicates_removed, Head>;
};

template<>
struct remove_duplicates<template_typelist<>>
{
    using type = template_typelist<>;
};

template<typename TList>
using remove_duplicates_t = typename remove_duplicates<TList>::type;

template<typename TList, template<template <typename...> typename > typename Pred>
struct any_of;

template<template<template <typename...> typename > typename Pred,
         template <typename...> typename... Ts>
struct any_of<template_typelist<Ts...>, Pred>
{
    static constexpr bool value = (Pred<Ts>::value || ...);
};

template<typename TList, template<template <typename...> typename > typename Pred>
constexpr bool any_of_v = any_of<TList, Pred>::value;

constexpr int default_index_value{-1};

template<typename TList, template <typename...> typename Element>
class get_index_by_type;

template<template <typename...> typename Element,
         template <typename...> typename Head,
         template <typename...> typename... Tails>
class get_index_by_type<template_typelist<Head, Tails...>, Element>
{
    static constexpr int temp = get_index_by_type<template_typelist<Tails...>, Element>::value;
public:
    static constexpr int value = (temp == default_index_value) ? default_index_value : 1 + temp;
};

template<template <typename...> typename Element,
         template <typename...> typename... Tails>
class get_index_by_type<template_typelist<Element, Tails...>, Element>
{
public:
    static constexpr int value{0};
};

template<template <typename...> typename Element>
class get_index_by_type<template_typelist<>, Element>
{
public:
    static constexpr int value{default_index_value};
};

template<typename TList, template <typename...> typename Element>
constexpr int get_index_by_type_v = get_index_by_type<TList, Element>::value;

template<typename TList, std::size_t Index>
struct get_type_by_index : nth_element<TList, Index>
{};

template<typename TList, std::size_t N, typename... Ts>
using get_type_by_index_tt = typename get_type_by_index<TList, N>::template type<Ts...>;
/*
 * replace_elment_with
 * */

} //tl
} //meta
} //infra
#endif
