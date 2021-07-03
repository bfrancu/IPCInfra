#ifndef TRANSPORT_TRAITS_HPP
#define TRANSPORT_TRAITS_HPP
#include "Devices/DeviceFactory.hpp"
#include "Devices/DeviceAddressFactory.hpp"
#include "Policies/ExporterPolicy.hpp"
#include "Policies/EventHandlingPolicy.hpp"
#include "Policies/DispatcherPolicy.hpp"
#include "TransportEndpoint.hpp"
#include "TransportDefinitions.h"
#include "ConnectionParameters.hpp"
#include "typelist.hpp"
#include "template_typelist.hpp"
#include "traits_utils.hpp"

namespace infra
{

    template<typename DeviceHost,
             typename EventHandlingPolicy,
             typename DispatcherPolicy,
             typename ClientServerRolePolicy,
             typename Listener,
             typename Storage,
             typename StateChangeCallbackDispatcher,
             typename TransportPolicies>
    struct generate_transport_endpoint;

    template<typename DeviceHost,
             template <typename...> typename EventHandlingPolicy,
             template <typename...> typename DispatcherPolicy,
             template <typename...> typename ClientServerRolePolicy,
             typename Listener,
             typename Storage,
             typename StateChangeCallbackDispatcher,
             typename TransportPolicies>
    struct generate_transport_endpoint<DeviceHost,
                                       meta::ttl::pack<EventHandlingPolicy>,
                                       meta::ttl::pack<DispatcherPolicy>,
                                       meta::ttl::pack<ClientServerRolePolicy>,
                                       Listener, 
                                       Storage,
                                       StateChangeCallbackDispatcher,
                                       TransportPolicies>
    {
        using type = PackHostT<TransportEndpoint<DeviceHost, EventHandlingPolicy, DispatcherPolicy,
                                                 ClientServerRolePolicy, Listener, Storage, StateChangeCallbackDispatcher>,
                               meta::ttl::remove_duplicates_t<TransportPolicies>>;
    };

    template<typename DeviceHost,
             typename EventHandlingPolicy,
             typename DispatcherPolicy,
             typename ClientServerRolePolicy,
             typename Listener,
             typename Storage,
             typename StateChangeCallbackDispatcher,
             typename TransportPolicies>
    using generate_transport_endpoint_t = typename generate_transport_endpoint<DeviceHost, EventHandlingPolicy,
                                                                               DispatcherPolicy, ClientServerRolePolicy,
                                                                               Listener, Storage,
                                                                               StateChangeCallbackDispatcher, TransportPolicies>::type;

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
    struct generate_device_types
    {
        using resource_handler_t = typename ClientTraits::ResourceHandler;
        using device_policies_t = typename ClientTraits::DevicePolicies;
        using device_t = typename DeviceFactory<DeviceTag>::template device_type<resource_handler_t>;
        using handle_t = typename device_t::handle_type;
        using device_address_t = typename DeviceAddressFactory<DeviceTag>::DeviceAddressT;
        using device_host_t = PackHostT<device_t, meta::ttl::remove_duplicates_t<device_policies_t>>;
    };

    template<typename ClientTraits>
    struct generate_endpoint_policies
    {
        using device_policies_t = typename ClientTraits::DevicePolicies;
        using export_policies_t = meta::ttl::intersect_t<typename ClientTraits::ExportPolicies, device_policies_t>;

        using transport_policies_t = traits::select_if_t<meta::ttl::is_empty<export_policies_t>, typename ClientTraits::TransportPolicies,
                                                         meta::ttl::push_back_t<typename ClientTraits::TransportPolicies,
                                                                                Exporter<export_policies_t>::template Policy>>;

        using endpoint_event_handling_policy_t = def::EventHandlingPolicy_or_default_t<ClientTraits, meta::ttl::pack<BaseEventHandlingPolicy>>;
        using endpoint_dispatcher_policy_t = def::DispatcherPolicy_or_default_t<ClientTraits, meta::ttl::pack<BaseDispatcherPolicy>>;
        using state_change_callback_dispatcher_t = def::StateChangeCallbackDispatcher_or_default_t<ClientTraits, SerialCallbackDispatcher>;
        using client_server_role_policy_t = typename ClientTraits::ClientServerRolePolicy;
        using listener_t = typename ClientTraits::Listener;
    };

    template<std::size_t DeviceTag, typename ClientTraits, typename = traits::select_if_t<std::conjunction<def::has_type_PeerTraits<ClientTraits>,
                                                                                                           def::has_type_EndpointStoragePolicy<ClientTraits>>,
                                                                                          std::true_type,
                                                                                          std::false_type>>
    class generate_endpoint_storage;

    template<std::size_t DeviceTag, typename ClientTraits>
    class generate_endpoint_storage<DeviceTag, ClientTraits, std::false_type>
    {
    public:
        using type = meta::tl::empty_type;
    };

    template<std::size_t DeviceTag, typename ClientTraits>
    class generate_endpoint_storage<DeviceTag, ClientTraits, std::true_type>
    {
        using peer_client_traits_t = typename ClientTraits::PeerTraits;
        using peer_endpoint_policies_t = generate_endpoint_policies<peer_client_traits_t>;
        using peer_device_types_t = generate_device_types<DeviceTag, peer_client_traits_t>;
        using handle_t = typename peer_device_types_t::handle_t;
        using peer_device_host_t = typename peer_device_types_t::device_host_t;
        using peer_endpoint_storage_t = meta::tl::empty_type;
        using peer_transport_endpoint_t = generate_transport_endpoint_t<peer_device_host_t,
                                                                        typename peer_endpoint_policies_t::endpoint_event_handling_policy_t,
                                                                        typename peer_endpoint_policies_t::endpoint_dispatcher_policy_t,
                                                                        typename peer_endpoint_policies_t::client_server_role_policy_t,
                                                                        typename peer_endpoint_policies_t::listener_t,
                                                                        peer_endpoint_storage_t,
                                                                        typename peer_endpoint_policies_t::state_change_callback_dispatcher_t,
                                                                        typename peer_endpoint_policies_t::transport_policies_t>;

        using endpoint_storage_key_t = def::EndpointStorageKey_or_default_t<ClientTraits, handle_t>;
        using endpoint_storage_policy_t = typename ClientTraits::EndpointStoragePolicy;

        template<typename Endpoint, typename Key, typename EndpointStoragePolicy>
        struct apply_types_to_storage_policy
        {
            using type = meta::tl::empty_type;
        };

        template<typename Endpoint, typename Key, template <typename... > typename EndpointStoragePolicy>
        struct apply_types_to_storage_policy<Endpoint, Key, meta::ttl::pack<EndpointStoragePolicy>>
        {
            using type = EndpointStoragePolicy<Endpoint, Key>;
        };

    public:
        using type = typename apply_types_to_storage_policy<peer_transport_endpoint_t, endpoint_storage_key_t, endpoint_storage_policy_t>::type;
    };


    template<std::size_t DeviceTag, typename ClientTraits>
    class transport_traits
    {
        using endpoint_policies_t = generate_endpoint_policies<ClientTraits>;
        using device_types_t = generate_device_types<DeviceTag, ClientTraits>;

    public:
        static constexpr std::size_t device_tag = DeviceTag;
        using resource_handler_t = typename device_types_t::resource_handler_t;
        using device_policies_t = typename device_types_t::device_policies_t;
        using device_t = typename device_types_t::device_t;
        using handle_t = typename device_types_t::handle_t;
        using device_address_t = typename device_types_t::device_address_t;
        using device_host_t = typename device_types_t::device_host_t;
        //using endpoint_storage_t = def::EndpointStorage_or_default_t<ClientTraits, meta::tl::empty_type>;
        using endpoint_storage_t = typename generate_endpoint_storage<DeviceTag, ClientTraits>::type;

        using transport_endpoint_t = generate_transport_endpoint_t<device_host_t,
                                                                   typename endpoint_policies_t::endpoint_event_handling_policy_t,
                                                                   typename endpoint_policies_t::endpoint_dispatcher_policy_t,
                                                                   typename endpoint_policies_t::client_server_role_policy_t,
                                                                   typename endpoint_policies_t::listener_t,
                                                                   endpoint_storage_t,
                                                                   typename endpoint_policies_t::state_change_callback_dispatcher_t,
                                                                   typename endpoint_policies_t::transport_policies_t>;
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


    template<std::size_t tag,
             typename ResourceHandler,
             typename DevicePolicies>
     struct device_generator
    {
        template<typename Device>
        using add_policies = PackHostT<Device, DevicePolicies>;

        using type = add_policies<typename Device2Type<tag>::template device_type<ResourceHandler>>;
    };

    template<typename ResourceHandler,
             typename DevicePolicies,
             template <std::size_t...> typename DeviceSet,
             std::size_t... tags>
    struct generate_device_typelist<ResourceHandler, DevicePolicies, DeviceSet<tags...>>
    {
        //template<typename Device> using add_policies = PackHostT<Device, DevicePolicies>;

        //using type = meta::tl::typelist<add_policies<typename Device2Type<tags...>::template device_type<ResourceHandler>>>;
        using type = meta::tl::generate_typelist_t<DeviceSet<tags...>, device_generator, ResourceHandler, DevicePolicies>;
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


    template<std::size_t tag, typename ClientTraits>
    struct endpoint_generator
    {
        using type = typename transport_traits<tag, ClientTraits>::transport_endpoint_t;
    };

    template<typename ClientTraits,
             template <std::size_t...> typename DeviceSet,
             std::size_t... tags>
    struct generate_endpoint_typelist<ClientTraits, DeviceSet<tags...>>
    {
        //using type = meta::tl::typelist<typename endpoint_generator<tags..., ClientTraits>::type>;
        using type  = meta::tl::generate_typelist_t<DeviceSet<tags...>, endpoint_generator, ClientTraits>;
    };

    template<typename EndpointTList>
    struct get_devices_from_endpoint_typelist;

    template<typename... Ts>
    struct get_devices_from_endpoint_typelist<meta::tl::typelist<Ts...>>
    {
        using type = meta::tl::typelist<typename Ts::Device...>;
    };

    template<typename T, typename = void>
    struct type_to_device_tag
    {
        static constexpr auto value{undefined_device_tag};
    };

    template<typename T>
    struct type_to_device_tag<T, std::enable_if_t<def::has_type_Device<T>::value>>
    {
        using HostDevice = typename T::Device;
        static constexpr auto value = Type2DeviceTag<typename UnpackHost<HostDevice>::ClientT>::value;
    };

} //infra

#endif
