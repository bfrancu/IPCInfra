#ifndef STORAGE_TRAITS_HPP
#define STORAGE_TRAITS_HPP
#include <array>

#include "TransportDefinitions.h"

namespace infra
{
namespace traits
{

template<typename T, typename = traits::select_if_t<def::has_type_key_t<T>,
                                                    std::true_type,
                                                    std::false_type >>
class has_member_getEndpointFor;

template<typename T>
class has_member_getEndpointFor<T, std::true_type>
{
    using Key = typename T::key_t;
    using SuccessReturnT = std::array<int, 2>;
    using FallbackReturnT = std::array<int, 1>;

    template<typename U, typename = decltype(std::declval<T>().getEndpointFor(std::declval<Key>()))>
    static constexpr SuccessReturnT test(void *) { return {}; }

    template<typename>
    static constexpr FallbackReturnT test(...) { return {}; }

public:
    static constexpr bool value = std::is_same_v<SuccessReturnT, decltype(test<T>(nullptr))>;
};

template<typename T>
class has_member_getEndpointFor<T, std::false_type> : public std::false_type
{};

template<typename T>
using is_endpoint_storage = traits::apply_predicates<T, def::has_member_store,
                                                        def::has_member_erase,
                                                        has_member_getEndpointFor,
                                                        def::has_type_endpoint_t,
                                                        def::has_type_key_t>;


template<typename T>
constexpr bool is_endpoint_storage_v = is_endpoint_storage<T>::value;

}//traits
}//infra

#endif
