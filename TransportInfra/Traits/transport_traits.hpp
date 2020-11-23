#ifndef TRANSPORT_TRAITS_HPP
#define TRANSPORT_TRAITS_HPP

#include "Devices/DeviceFactory.hpp"
#include "TransportEndpoint.hpp"
#include "ConnectionParameters.hpp"

namespace infra
{
    template<typename ResourceHandler,
             typename ConnectionParameters,
             template<typename...> typename... DevicePolicies>
    struct transport_traits
    {
        using device_t = typename DeviceFactory<ConnectionParameters::dev_type>::template device_type<ResourceHandler>;
        using handle_t = typename device_t::handle_type;
        using device_host_t = Host<device_t, DevicePolicies...>;
        using policies_holder_t = typename ConnectionParameters::transport_policies_pack;
        using transport_endpoint_t = typename policies_holder_t::template AssembledClientT<TransportEndpoint<device_host_t>>;
    };

} //infra

#endif
