#ifndef DEVICE_TYPE_SELECTOR_HPP
#define DEVICE_TYPE_SELECTOR_HPP
#include "default_traits.hpp"
#include "typelist.hpp"
#include "non_typelist.hpp"
#include "Devices/DeviceAddressFactory.hpp"
#include "Connector.hpp"
#include "Traits/transport_traits.hpp"
#include "Traits/device_traits.hpp"
#include "ConfigurationBook.h"
#include "ConnectionParameters.hpp"

namespace infra
{

    template<std::size_t DeviceTag,
             typename ResourceHandler,
             typename DevicePolicies,
             typename TransportPolicies,
             typename Connector,
             typename CompletionCallback>
    constexpr bool forwardConnect(const config::ConfigurationBook & book, std::string_view section,
                           Connector & connector, CompletionCallback && cb)
    {
            using dev_addr_t = typename DeviceAddressFactory<DeviceTag>::DeviceAddressT; 
            ConnectionParameters<DeviceTag, dev_addr_t> conn_params;
            conn_params.address = DeviceAddressFactory<DeviceTag>::createAddress(book, section);
            return connector.template setup<ResourceHandler, DevicePolicies, TransportPolicies>
                                            (conn_params, std::forward<CompletionCallback>(cb));
    }

// delegate the selection of device address type and device type to this class
// based on non type tag
// the only reason I made this a template after the non-type tag is to allow the 
// partial specialization for non default tags that are not covered by EDeviceType 
template<int tag = static_cast<int>(EDeviceType::E_UNDEFINED_DEVICE), typename Enable = void>
class DeviceTypeSelector
{
public:
    template<typename ResourceHandler,
             typename DevicePolicies,
             typename TransportPolicies,
             typename Connector,
             typename CompletionCallback>
    static bool connect(std::size_t device_tag, const infra::config::ConfigurationBook & book, std::string_view section,
                         Connector & connector, CompletionCallback && cb)
    {
        bool ret{false};
        switch (device_tag)
        {
        case ipv4_strm_tag:
        {
            ret = forwardConnect<ipv4_strm_tag, ResourceHandler, DevicePolicies, TransportPolicies>
                          (book, section, connector, std::forward<CompletionCallback>(cb));
            break;
        }
        case static_cast<std::size_t>(EDeviceType::E_IPV4_UDP_SOCKET_DEVICE):
            break;
        case static_cast<std::size_t>(EDeviceType::E_IPV6_TCP_SOCKET_DEVICE):
            break;
        case static_cast<std::size_t>(EDeviceType::E_IPV6_UDP_SOCKET_DEVICE):
            break;
        case static_cast<std::size_t>(EDeviceType::E_UNIX_STREAM_SOCKET_DEVICE):
            break;
        case static_cast<std::size_t>(EDeviceType::E_UNIX_DGRAM_SOCKET_DEVICE):
            break;
        case static_cast<std::size_t>(EDeviceType::E_READING_FIFO_DEVICE):
            break;
        case static_cast<std::size_t>(EDeviceType::E_WRITING_FIFO_DEVICE):
            break;
        }
        return ret;
    }
};

template<typename DeviceSet, typename Enable = void>
struct RuntimeTransportTraitsResolution;

template<template<std::size_t...> typename DeviceSet>
struct RuntimeTransportTraitsResolution<DeviceSet<>>;

template<std::size_t... tags>
struct RuntimeTransportTraitsResolution<meta::ntl::non_typelist<tags...>>
{
    using static_tags_list = meta::ntl::non_typelist<tags...>;

    template<std::size_t TagIndex, typename ClientTraits, typename Callable, typename... Args>
    static decltype(auto) forward_helper(std::size_t device_tag, Callable && callable, Args&&... args)
    {
        using namespace meta;
        constexpr std::size_t static_tag = ntl::nth_element_v<static_tags_list, TagIndex>;

        using transport_traits = transport_traits<static_tag, ClientTraits>;
        using return_t = decltype(std::declval<Callable>()(std::declval<meta::tl::pack<transport_traits>>(), std::declval<Args&&>()...));

        if (static_tag == device_tag){
            return callable(meta::tl::pack<transport_traits>{}, std::forward<Args>(args)...);
        }

        if constexpr (TagIndex < ntl::size_v<static_tags_list> - 1){
            return forward_helper<TagIndex+1, ClientTraits>(device_tag, std::forward<Callable>(callable), std::forward<Args>(args)...);
        }

        return static_cast<return_t>(meta::traits::default_value<return_t>::value);
    }

    template<typename ClientTraits, typename Callable, typename... Args>
    static decltype(auto) forward(std::size_t device_tag, Callable && callable, Args&&... args)
    {
        return forward_helper<0, ClientTraits>(device_tag, std::forward<Callable>(callable), std::forward<Args>(args)...);
    }
};

template<>
struct RuntimeTransportTraitsResolution<default_device_set, void>
{
    template<typename ClientTraits, typename Callable, typename... Args>
    static decltype(auto) forward(std::size_t device_tag, Callable && callable, Args&&... args)
    {
        using default_transport_traits_t = transport_traits<ipv4_strm_tag, ClientTraits>;
        using return_t = decltype(std::declval<Callable>()(std::declval<meta::tl::pack<default_transport_traits_t>>(), std::declval<Args&&>()...));

        switch (device_tag)
        {
            case ipv4_strm_tag:
            {
                using transport_traits = default_transport_traits_t;
                return callable(meta::tl::pack<transport_traits>{}, std::forward<Args>(args)...);
            }
            case ipv4_dgram_tag:
            {
                using transport_traits = transport_traits<ipv4_dgram_tag, ClientTraits>;
                return callable(meta::tl::pack<transport_traits>{}, std::forward<Args>(args)...);
            }
            case ipv6_strm_tag:
            {
                using transport_traits = transport_traits<ipv6_strm_tag, ClientTraits>;
                return callable(meta::tl::pack<transport_traits>{}, std::forward<Args>(args)...);
            }
            case ipv6_dgram_tag:
            {
                using transport_traits = transport_traits<ipv6_dgram_tag, ClientTraits>;
                return callable(meta::tl::pack<transport_traits>{}, std::forward<Args>(args)...);
            }
            case unx_strm_tag:
            {
                using transport_traits = transport_traits<unx_strm_tag, ClientTraits>;
                return callable(meta::tl::pack<transport_traits>{}, std::forward<Args>(args)...);
            }
            case unx_dgram_tag:
            {
                using transport_traits = transport_traits<unx_dgram_tag, ClientTraits>;
                return callable(meta::tl::pack<transport_traits>{}, std::forward<Args>(args)...);
            }
            case read_fifo_tag:
            {
                using transport_traits = transport_traits<read_fifo_tag, ClientTraits>;
                return callable(meta::tl::pack<transport_traits>{}, std::forward<Args>(args)...);
            }
            case write_fifo_tag:
            {
                using transport_traits = transport_traits<write_fifo_tag, ClientTraits>;
                return callable(meta::tl::pack<transport_traits>{}, std::forward<Args>(args)...);
            }
        }
        return static_cast<return_t>(meta::traits::default_value<return_t>::value);
    }
};

class ConnectionInitializerAdapter
{
public:
    template<typename ClientTraits, std::size_t DeviceTag, typename ConnectionInitializer, typename... Args>
    static constexpr bool connect(const infra::config::ConfigurationBook & book, std::string_view section,
                                  ConnectionInitializer & connection_initializer, Args&&... args)
    {
        std::cout << "ConnectionInitializerAdapter::connect<DeviceTag>()\n";
        using transport_traits = transport_traits<DeviceTag, ClientTraits>;
        return forwardConnect<transport_traits>(book, section, connection_initializer, std::forward<Args>(args)...);
    }

    template<typename ClientTraits, typename ConnectionInitializer, typename... Args>
    static bool connect(std::size_t device_tag, const infra::config::ConfigurationBook & book,
                        std::string_view section, ConnectionInitializer & connection_initializer, Args&&... args)
    {
        std::cout << "ConnectionInitializerAdapter::connect(DeviceTag)\n";
        using device_set_t = typename ClientTraits::DeviceSet;
        auto callable = [] (auto traits_pack, auto&&... fwargs) {
            return forwardConnect(traits_pack, std::forward<decltype(fwargs)>(fwargs)...);
        };
        return RuntimeTransportTraitsResolution<device_set_t>::template
            forward<ClientTraits>(device_tag, callable, book, section, connection_initializer, std::forward<Args>(args)...);
    }

private:
   template<typename TransportTraits, typename ConnectionInitializer, typename... Args>
   static constexpr bool forwardConnect(const infra::config::ConfigurationBook & book, std::string_view section,
                                        ConnectionInitializer & connection_initializer, Args&&... args)
   {
       typename TransportTraits::device_address_t dev_addr = DeviceAddressFactory<TransportTraits::device_tag>::createAddress(book, section);
       std::cout << "ConnectionInitializerAdapter::forwardConnect<TransportTraits>() device address: " << dev_addr << "\n";
       if(dev_addr.empty()) return false;

       return connection_initializer.template setup<TransportTraits>(dev_addr, std::forward<Args>(args)...);
   }
   
   template<typename TransportTraits, typename ConnectionInitializer, typename... Args>
   static constexpr bool forwardConnect(meta::tl::pack<TransportTraits>, const infra::config::ConfigurationBook & book, 
                                        std::string_view section, ConnectionInitializer & connection_initializer, Args&&... args)
   {
       return forwardConnect<TransportTraits>(book, section, connection_initializer, std::forward<Args>(args)...);
   }
};

}//infra
    
#endif
