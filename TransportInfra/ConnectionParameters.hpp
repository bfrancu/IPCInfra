#ifndef CONNECTION_PARAMETERS_HPP
#define CONNECTION_PARAMETERS_HPP
#include <cstdio>

#include "PoliciesHolder.hpp"

namespace infra
{

template <std::size_t DeviceTag,
          typename DeviceAddress,
          template<typename... > typename... TransportPolicies>
struct ConnectionParameters1
{
    constexpr static std::size_t dev_type{DeviceTag};
    using transport_policies_pack = PoliciesHolder<TransportPolicies...>;
    DeviceAddress addr;
};

template<std::size_t DeviceTag,
         typename DeviceAddress>
struct ConnectionParameters
{
    DeviceAddress address;
};

} //infra
#endif
