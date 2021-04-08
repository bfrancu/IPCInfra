#ifndef TYPE_LIST_HPP
#define TYPE_LIST_HPP
#include <tuple>
#include <variant>

namespace infra
{
namespace meta
{
namespace tl
{

template<typename... Ts>
struct typelist
{};

struct empty_type{};

template<typename Element>
struct pack{};

template<typename TList>
struct is_empty
{
    static constexpr bool value = false;
};

template<>
struct is_empty<typelist<>>
{
    static constexpr bool value = true;
};

template<typename TList>
constexpr bool is_empty_v = is_empty<TList>::value;

template<typename TList>
struct size;

template<typename... Ts>
struct size<typelist<Ts...>>
{
    static constexpr std::size_t value = sizeof...(Ts);
};

template<typename TList>
constexpr std::size_t size_v = size<TList>::value;

template<typename TList>
struct front;

template<typename Head, typename... Tails>
struct front<typelist<Head, Tails...>>
{
    using type = Head;
};

template<typename TList>
using front_t = typename front<TList>::type;

template<typename TList>
struct pop_front;

template<typename Head, typename... Tails>
struct pop_front<typelist<Head, Tails...>>
{
   using type = typelist<Tails...>;
};

template<typename TList>
using pop_front_t = typename pop_front<TList>::type;

template<typename TList, typename NewElement>
struct push_front;

template<typename NewElement, typename... Tails>
struct push_front<typelist<Tails...>, NewElement>
{
    using type = typelist<NewElement, Tails...>;
};

template<typename TList, typename NewElement>
using push_front_t = typename push_front<TList, NewElement>::type;

template<typename TList, typename NewElement>
struct push_back;

template<typename NewElement, typename... Tails>
struct push_back<typelist<Tails...>, NewElement>
{
    using type = typelist<Tails..., NewElement>;
};

template<typename TList, typename NewElement>
using push_back_t = typename push_back<TList, NewElement>::type;

template<typename TList, std::size_t N>
struct nth_element : nth_element<pop_front_t<TList>, N-1>
{};

template<typename TList>
struct nth_element<TList, 0> : front<TList>
{};

template<std::size_t N>
struct nth_element<typelist<>, N>
{
    using type = empty_type;
};

template<typename TList, std::size_t N>
using nth_element_t = typename nth_element<TList, N>::type;

template<typename TList, typename Element>
struct erase_rec;

template<typename Head, typename Element, typename... Tails>
struct erase_rec<typelist<Head, Tails...>, Element>
{
   using tails = typename erase_rec<typelist<Tails...>, Element>::type;
   using type = push_front_t<tails, Head>;
};

template<typename Element, typename... Tails>
struct erase_rec<typelist<Element, Tails...>, Element>
{
   using type = typename erase_rec<typelist<Tails...>, Element>::type;
};

template<typename Element>
struct erase_rec<typelist<>, Element>
{
    using type = typelist<>;
};

template<typename TList, typename Element>
using erase_rec_t = typename erase_rec<TList, Element>::type;

template<typename TList>
struct to_tuple;

template<typename... Ts>
struct to_tuple<typelist<Ts...>>
{
   using type = std::tuple<Ts...>;  
};

template<typename TList>
using to_tuple_t = typename to_tuple<TList>::type;

template<typename TList>
struct to_variant;

template<typename... Ts>
struct to_variant<typelist<Ts...>>
{
   using type = std::variant<Ts...>;  
};

template<typename TList>
using to_variant_t = typename to_variant<TList>::type;

template<typename Tuple>
struct to_tlist;

template<typename... Ts>
struct to_tlist<std::tuple<Ts...>>
{
    using type = typelist<Ts...>;
};

template<typename... Ts>
struct to_tlist<std::variant<Ts...>>
{
    using type = typelist<Ts...>;
};

template<typename Tuple>
using to_tlist_t = typename to_tlist<Tuple>::type;

template<typename TList, typename T>
struct contains;

template<typename T, typename... Ts>
struct contains<typelist<Ts...>, T>
{
    static constexpr bool value = std::disjunction_v<std::is_same<Ts, T>...>;
};

template<typename T, template <typename... > typename Container, typename... Ts>
struct contains<Container<Ts...>, T>
{
    static constexpr bool value = contains<to_tlist_t<Container<Ts...>>, T>::value;
};

template<typename TList, typename T>
constexpr bool contains_v = contains<TList, T>::value;

template<typename... TLists>
struct concat
{
    using concat_tuples = decltype(std::tuple_cat(std::declval<to_tuple_t<TLists>>()...));
    using type = to_tlist_t<concat_tuples>;
};

template<typename... TLists>
using concat_t = typename concat<TLists...>::type;

template<bool>
struct zero_or_one
{
    template<typename Element>
    struct helper
    {
        using type = typelist<>;
    };
};

template<>
struct zero_or_one<true>
{
    template<typename Element>
    struct helper
    {
        using type = typelist<Element>;
    };
};

template<template <typename...> typename Pred, typename TList, bool FilterIn = true>
struct filter;

template<bool FilterIn, template <typename...> typename Pred,
         typename... Ts>
struct filter<Pred, typelist<Ts...>, FilterIn>
{
    using type = concat_t<typename zero_or_one<(FilterIn ? Pred<Ts>::value
                                                         : !Pred<Ts>::value)>::template helper<Ts>::type... >;
};

template<template <typename...> typename Pred, typename TList, bool FilterIn>
using filter_t = typename filter<Pred, TList, FilterIn>::type;

template<typename TList, typename Element>
struct erase;

template<typename Element, typename... Ts>
struct erase<typelist<Ts...>, Element>
{
    template<typename E>
    struct pred : std::is_same<E, Element>
    {};

    using type = filter_t<pred, typelist<Ts...>, false>;
};

template<typename Tlist, typename Element>
using erase_t = typename erase<Tlist, Element>::type;

template<typename TList>
struct remove_duplicates;

template<typename Head, typename... Tails>
struct remove_duplicates<typelist<Head, Tails...>>
{
    using tails_filtered_by_head = erase_t<typelist<Tails...>, Head>; 
    using tails_duplicates_removed = typename remove_duplicates<tails_filtered_by_head>::type;
    using type = push_front_t<tails_duplicates_removed, Head>;
};

template<>
struct remove_duplicates<typelist<>>
{
    using type = typelist<>;
};

template<typename TList>
using remove_duplicates_t = typename remove_duplicates<TList>::type;

#define DEFINE_GET_MEMBER_TYPE_BY_INDEX(Member)                                \
template<typename TList, std::size_t N>                                        \
struct get_##Member##_type_by_index                                            \
{                                                                              \
    using nth_element = nth_element_t<TList, N>;                               \
    using type = std::conditional_t<std::is_same_v<nth_element, empty_type>,   \
                                    empty_type,                                \
                                    typename nth_element::Member>;             \
} //; intentionally skipped

} //tl
} //meta
} //infra
#endif
