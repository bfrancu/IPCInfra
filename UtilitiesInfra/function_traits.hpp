#ifndef FUNCTION_TRAITS_HPP
#define FUNCTION_TRAITS_HPP
#include <type_traits>
#include <typelist.hpp>

namespace infra
{
namespace meta
{
namespace traits
{
template<typename> struct return_type_from_function;

template<typename T, typename... Args>
struct return_type_from_function<T(Args...)>
{
    using type = T;
};

template<typename T, typename... Args>
struct return_type_from_function<T(Args...)const>
{
    using type = T;
};

template<typename T, typename... Args>
struct return_type_from_function<T(Args...) &&>
{
    using type = T;
};

template<typename T, typename... Args>
struct return_type_from_function<T (*)(Args...)>
{
    using type = T;
};

template<typename> struct types_from_class_member;

template<typename Parent, typename Member>
struct types_from_class_member<Member Parent::*>
{
    using parent_type = Parent;
    using member_type = Member;
};

template<typename>
struct is_pointer_to_member : std::false_type {};

template<typename C, typename M>
struct is_pointer_to_member<M C::*> : std::true_type {};

template<typename>
struct is_pointer_to_static_func : std::false_type {};

template<typename M, typename...Args>
struct is_pointer_to_static_func<M (*)(Args...)> : std::true_type {};

template<typename PM>
constexpr bool is_pointer_to_member_v = is_pointer_to_member<PM>::value;

template<typename PS>
constexpr bool is_pointer_to_static_func_v = is_pointer_to_static_func<PS>::value;

template<auto V>
using pointer_to_member_constraint = std::enable_if_t<is_pointer_to_member_v<decltype(V)>>;

template<auto V>
using pointer_to_static_constraint = std::enable_if_t<is_pointer_to_static_func_v<decltype(V)>>;

template<auto V, typename = void>
struct return_type_from_member_function;

template<auto StaticMember>
struct return_type_from_member_function<StaticMember, pointer_to_static_constraint<StaticMember>>
{
    using type = typename traits::return_type_from_function<decltype(StaticMember)>::type;
};

template<auto PtrToMemberFunc>
struct return_type_from_member_function<PtrToMemberFunc, traits::pointer_to_member_constraint<PtrToMemberFunc>>
{
    using ptr_to_member_t  = decltype(PtrToMemberFunc);
    using subject_t        = typename traits::types_from_class_member<ptr_to_member_t>::parent_type;
    using member_t         = typename traits::types_from_class_member<ptr_to_member_t>::member_type;
    using type             = typename traits::return_type_from_function<member_t>::type;
};

template<auto V>
using return_type_from_member_function_t = typename return_type_from_member_function<V>::type;

#define DEFINE_RETURN_TYPES_FROM_MEMBER(Member)                                                                      \
template<typename TL>                                                                                             \
struct return_types_from_##Member;                                                                                   \
                                                                                                                     \
template<typename... Ts>                                                                                             \
struct return_types_from_##Member<infra::meta::tl::typelist<Ts...>>                                                  \
{                                                                                                                    \
    using type = infra::meta::tl::typelist<infra::meta::traits::return_type_from_member_function_t<&Ts::Member>...>; \
};                                                                                                                   \
                                                                                                                     \
template<typename TL>                                                                                             \
using return_types_from_##Member##_t = typename return_types_from_##Member<TL>::type//; intentionally skipped




} //traits
} //meta
} //infra
#endif
