#ifndef TRANSPORT_TRAITS_HPP
#define TRANSPORT_TRAITS_HPP
#include "Devices/DeviceFactory.hpp"
#include "Devices/DeviceAddressFactory.hpp"
#include "Policies/ExporterPolicy.hpp"
#include "TransportEndpoint.hpp"
#include "ConnectionParameters.hpp"
#include "typelist.hpp"
#include "template_typelist.hpp"
#include "traits_utils.hpp"

namespace infra
{

    template<typename DeviceHost,
             typename Listener,
             typename TransportPolicies,
             typename EventHandlingPolicy,
             typename DispatcherPolicy>
    struct generate_transport_endpoint;

    template<typename DeviceHost,
             typename Listener,
             typename TransportPolicies,
             template <typename...> typename EventHandlingPolicy,
             template <typename...> typename DispatcherPolicy>
    struct generate_transport_endpoint<DeviceHost, Listener, TransportPolicies,
                                       meta::ttl::pack<EventHandlingPolicy>,
                                       meta::ttl::pack<DispatcherPolicy>>
    {
        using type = PackHostT<TransportEndpoint<DeviceHost, EventHandlingPolicy, DispatcherPolicy, Listener>,
                                 meta::ttl::remove_duplicates_t<TransportPolicies>>;
    };

    template<typename DeviceHost,
             typename Listener,
             typename TransportPolicies,
             typename EventHandlingPolicy,
             typename DispatcherPolicy>
    using generate_transport_endpoint_t = typename generate_transport_endpoint<DeviceHost, Listener, TransportPolicies, EventHandlingPolicy, DispatcherPolicy>::type;

    template<typename TTList>
    struct generate_hierarchy;

    template<template <typename...> typename... Policies>
    struct generate_hierarchy<meta::ttl::template_typelist<Policies...>>
    {
        template<typename Host, typename Device>
        class policy_type : public Policies<Host, Device>...
        {};
    };

    template<>
    struct generate_hierarchy<meta::ttl::template_typelist<>>
    {
       template<typename Host, typename Device>
       class policy_type {};
    };

    template<typename...>
    struct TestPolicy{};

    template<std::size_t DeviceTag, typename ClientTraits>
    struct transport_traits
    {
        static constexpr std::size_t device_tag = DeviceTag;
        using resource_handler_t = typename ClientTraits::ResourceHandler;
        using device_policies_t = typename ClientTraits::DevicePolicies;
        using export_policies_t = meta::ttl::intersect_t<typename ClientTraits::ExportPolicies, device_policies_t>;
        using device_t = typename DeviceFactory<device_tag>::template device_type<typename ClientTraits::ResourceHandler>;
        using handle_t = typename device_t::handle_type;
        using device_address_t = typename DeviceAddressFactory<device_tag>::DeviceAddressT; 
        using device_host_t = PackHostT<device_t, meta::ttl::remove_duplicates_t<typename ClientTraits::DevicePolicies>>;

        using transport_policies_t = traits::select_if_t<meta::ttl::is_empty<export_policies_t>, typename ClientTraits::TransportPolicies,
                                                         meta::ttl::push_back_t<typename ClientTraits::TransportPolicies,
                                                                                Exporter<export_policies_t>::template Policy>>;

        using transport_endpoint_t = generate_transport_endpoint_t<device_host_t, typename ClientTraits::Listener,
                                                                                  transport_policies_t,
                                                                                  typename ClientTraits::EventHandlingPolicy,
                                                                                  typename ClientTraits::DispatcherPolicy>;
    };

    template<typename ResourceHandler,
             typename DevicePolicies,
             typename DeviceSet = default_device_set,
             typename Enable = void>
    struct generate_device_typelist;

    template<typename ResourceHandler,
             typename DevicePolicies>
    struct generate_device_typelist<ResourceHandler, DevicePolicies, default_device_set>
    {
        template<typename Device>
        using add_policies = PackHostT<Device, DevicePolicies>;

        using type = meta::tl::typelist<add_policies<typename Device2Type<ipv4_strm_tag>::template device_type<ResourceHandler>>,
                                        add_policies<typename Device2Type<ipv4_dgram_tag>::template device_type<ResourceHandler>>,
                                        add_policies<typename Device2Type<ipv6_strm_tag>::template device_type<ResourceHandler>>,
                                        add_policies<typename Device2Type<ipv6_dgram_tag>::template device_type<ResourceHandler>>,
                                        add_policies<typename Device2Type<unx_strm_tag>::template device_type<ResourceHandler>>,
                                        add_policies<typename Device2Type<unx_dgram_tag>::template device_type<ResourceHandler>>,
                                        add_policies<typename Device2Type<read_fifo_tag>::template device_type<ResourceHandler>>,
                                        add_policies<typename Device2Type<write_fifo_tag>::template device_type<ResourceHandler>>
                                       >;
    };

    template<typename ClientTraits,
             typename DeviceSet = typename ClientTraits::DeviceSet,
             typename Enable = void>
    struct generate_endpoint_typelist;

    template<typename ClientTraits>
    struct generate_endpoint_typelist<ClientTraits, default_device_set>
    {
        using type = meta::tl::typelist<typename transport_traits<ipv4_strm_tag, ClientTraits>::transport_endpoint_t,
                                        typename transport_traits<ipv4_dgram_tag, ClientTraits>::transport_endpoint_t,
                                        typename transport_traits<ipv6_strm_tag, ClientTraits>::transport_endpoint_t,
                                        typename transport_traits<ipv6_dgram_tag, ClientTraits>::transport_endpoint_t,
                                        typename transport_traits<unx_strm_tag, ClientTraits>::transport_endpoint_t,
                                        typename transport_traits<unx_dgram_tag, ClientTraits>::transport_endpoint_t,
                                        typename transport_traits<read_fifo_tag, ClientTraits>::transport_endpoint_t,
                                        typename transport_traits<write_fifo_tag, ClientTraits>::transport_endpoint_t
                                        >;
    };

    template<typename EndpointTList>
    struct get_devices_from_endpoint_typelist;

    template<typename... Ts>
    struct get_devices_from_endpoint_typelist<meta::tl::typelist<Ts...>>
    {
        using type = meta::tl::typelist<typename Ts::Device...>;
    };

} //infra

#endif
