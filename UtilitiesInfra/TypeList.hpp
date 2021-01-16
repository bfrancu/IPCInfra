#ifndef TYPELIST_HPP
#define TYPELIST_HPP
#include "Host.hpp"

namespace infra
{
template<typename... Ts>
struct Typelist
{};

template<template<typename... > typename... Ts>
struct TemplateTypelist
{};


template<typename TList>
struct IsEmpty
{
    static constexpr bool value = false;
};

template<>
struct IsEmpty<TemplateTypelist<>> 
{
    static constexpr bool value = true;
};

template<>
struct IsEmpty<Typelist<>>
{
    static constexpr bool value = true;
};

template<typename TList>
struct Front
{};

template<typename Head, typename... Tails>
struct Front<Typelist<Head, Tails...>>
{
    using Type = Head;
};

template<template <typename...> typename Head, template <typename...> typename... Tails>
struct Front<TemplateTypelist<Head, Tails...>>
{
    template<typename... Ts>
    using Type = Head<Ts...>;
};

template<typename TList>
struct PopFront;

template<typename Head, typename... Tails>
struct PopFront<Typelist<Head, Tails...>>
{
    using Type = Typelist<Tails...>;
};

template<template <typename...> typename Head, template <typename...> typename... Tails>
struct PopFront<TemplateTypelist<Head, Tails...>>
{
    using Type = TemplateTypelist<Tails...>;
};

template<typename TList, typename NewElement>
struct PushBack;






} //infra

#endif
