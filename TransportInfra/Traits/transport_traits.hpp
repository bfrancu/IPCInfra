#ifndef TRANSPORT_TRAITS_HPP
#define TRANSPORT_TRAITS_HPP
#include "Devices/DeviceFactory.hpp"
#include "Devices/DeviceAddressFactory.hpp"
#include "TransportEndpoint.hpp"
#include "ConnectionParameters.hpp"
#include "typelist.hpp"
#include "template_typelist.hpp"

namespace infra
{

    template<typename DeviceHost,
             typename Listener,
             typename TransportPolicies,
             typename EventHandlingPolicy>
    struct generate_transport_endpoint;

    template<typename DeviceHost,
             typename Listener,
             typename TransportPolicies,
             template <typename...> typename EventHandlingPolicy>
    struct generate_transport_endpoint<DeviceHost, Listener, TransportPolicies,
                                       meta::ttl::pack<EventHandlingPolicy>>
    {
        using type = PackHostT<TransportEndpoint<DeviceHost, EventHandlingPolicy, Listener>,
                                 meta::ttl::remove_duplicates_t<TransportPolicies>>;
    };

    template<typename DeviceHost,
             typename Listener,
             typename TransportPolicies,
             typename EventHandlingPolicy>
    using generate_transport_endpoint_t = typename generate_transport_endpoint<DeviceHost, Listener, 
                                                                               TransportPolicies, EventHandlingPolicy>::type;

    template<std::size_t DeviceTag, typename ClientTraits>
    struct transport_traits
    {
        static constexpr std::size_t device_tag = DeviceTag;
        using resource_handler_t = typename ClientTraits::ResourceHandler;
        using device_policies_t = typename ClientTraits::DevicePolicies;
        using device_t = typename DeviceFactory<device_tag>::template device_type<typename ClientTraits::ResourceHandler>;
        using handle_t = typename device_t::handle_type;
        using device_address_t = typename DeviceAddressFactory<device_tag>::DeviceAddressT; 
        using device_host_t = PackHostT<device_t, meta::ttl::remove_duplicates_t<typename ClientTraits::DevicePolicies>>;
        //using transport_endpoint_t = PackHostT<TransportEndpoint<device_host_t, Listener>, meta::ttl::remove_duplicates_t<TransportPolicies>>;
        using transport_endpoint_t = generate_transport_endpoint_t<device_host_t, typename ClientTraits::Listener,
                                                                                  typename ClientTraits::TransportPolicies,
                                                                                  typename ClientTraits::EventHandlingPolicy>;
    };

    template<typename ResourceHandler,
             typename DevicePolicies,
             typename Enable = void>
    struct generate_device_typelist
    {
        template<typename Device>
        using add_policies = PackHostT<Device, DevicePolicies>;

        using type = meta::tl::typelist<add_policies<typename Device2Type<ipv4_dgram_tag>::template device_type<ResourceHandler>>,
                                        add_policies<typename Device2Type<ipv6_strm_tag>::template device_type<ResourceHandler>>,
                                        add_policies<typename Device2Type<ipv6_dgram_tag>::template device_type<ResourceHandler>>,
                                        add_policies<typename Device2Type<unx_strm_tag>::template device_type<ResourceHandler>>,
                                        add_policies<typename Device2Type<unx_dgram_tag>::template device_type<ResourceHandler>>,
                                        add_policies<typename Device2Type<read_fifo_tag>::template device_type<ResourceHandler>>,
                                        add_policies<typename Device2Type<write_fifo_tag>::template device_type<ResourceHandler>>
                                       >;
    };
} //infra

#endif
