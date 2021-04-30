#ifndef TRAITS_UTILS_HPP
#define TRAITS_UTILS_HPP
#include <type_traits>

namespace infra
{
namespace traits
{


#define DEFINE_HAS_TYPE(MemType)                                      \
  template<typename T, typename = std::void_t<>>                      \
  struct has_type_##MemType                                           \
    : std::false_type {};                                             \
                                                                      \
  template<typename T>                                                \
  struct has_type_##MemType<T, std::void_t<typename T::MemType>>      \
   : std::true_type {} // ; intentionally skipped


#define DEFINE_HAS_MEMBER1(Member)                                    \
  template<typename T, typename = std::void_t<>>                      \
  struct HasMemberT_##Member                                          \
    : std::false_type {};                                             \
                                                                      \
  template<typename T>                                                \
  struct HasMemberT_##Member<T, std::void_t<decltype(&T::Member)>>    \
    : std::true_type {} // ; intentionally skipped 

  
#define DEFINE_HAS_MEMBER(Member)                 \
template<typename T, typename = std::void_t<>>    \
struct has_member_##Member : std::false_type {};  \
                                                  \
template<typename T>                              \
struct has_member_##Member<T, std::void_t<decltype(&T::Member)>> : std::true_type {} //; intentionally skipped


template<typename Pred, typename Type1, typename Type2>
struct select_if
{

template<typename T1, typename T2, bool>
struct select_if_helper { using type = T2; };

template<typename T1, typename T2>
struct select_if_helper<T1, T2, true> { using type = T1; };

using type = typename select_if_helper<Type1, Type2, Pred::value>::type;
};

template<typename Pred, typename T1, typename T2>
using select_if_t = typename select_if<Pred, T1, T2>::type;

template<typename T, typename... Ts>
struct first_of
{
    using type = T;
};

template<typename T, typename... Ts>
struct are_related : std::false_type
{};

template<typename T, typename U>
struct are_related<T, U> : std::is_base_of<T, std::decay_t<U>>
{};

template<typename T, typename... Ts>
constexpr bool are_related_v = are_related<T, Ts...>::value;

}//traits

} //infra
#endif
