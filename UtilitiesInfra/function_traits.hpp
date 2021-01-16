#ifndef FUNCTION_TRAITS_HPP
#define FUNCTION_TRAITS_HPP

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

} //traits
} //meta
} //infra
#endif
