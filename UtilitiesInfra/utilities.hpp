#ifndef UTILITIES_HPP
#define UTILITIES_HPP
#include <type_traits>

namespace infra
{

namespace utils
{

template<typename T>
struct TypeT{
  using Type = T;
};

template<typename T>
constexpr auto type = TypeT<T>{};

template <typename T>
T valueT(TypeT<T>);

inline constexpr
auto typeEquals = [] (auto original_type){
    return [] (auto compared_type){
        return std::is_same_v<decltype(valueT(compared_type)), decltype(valueT(original_type))>;
    };
};


template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) {
    return static_cast<typename std::underlying_type<E>::type>(e);
}


} //utils
} // infra

#endif // UTILITIES_HPP
