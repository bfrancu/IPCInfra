#ifndef DEFAULT_TRAITS_HPP
#define DEFAULT_TRAITS_HPP

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

}//traits
}//meta
}//infra
#endif
