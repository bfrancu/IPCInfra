#ifndef DEFAULT_TRAITS_HPP
#define DEFAULT_TRAITS_HPP
#include <cstdint>

namespace infra
{
namespace meta
{
namespace traits
{

template<typename T, typename Enable = void>
struct default_value
{
    static constexpr T value{};
};

template<>
struct default_value<bool, void>
{
    static constexpr bool value{false};
};

template<>
struct default_value<int, void>
{
     static constexpr int value = -1;
};

template<>
struct default_value<std::size_t>
{
    static constexpr std::size_t value = 0;
};

}//traits
}//meta
}//infra
#endif
