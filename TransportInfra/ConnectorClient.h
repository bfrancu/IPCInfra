#ifndef CONNECTOR_CLIENT_H
#define CONNECTOR_CLIENT_H
#include <string>
#include <unordered_map>

#include "Host.hpp"
#include "Reactor/EpollDemultiplexer.h"
#include "template_typelist.hpp"
#include "Devices/DeviceDefinitions.h"
#include "Policies/ResourceStatusPolicy.hpp"
#include "Policies/IOPolicy.hpp"
#include "Policies/ConnectionPolicy.hpp"
#include "Policies/EventHandlingPolicy.hpp"
#include "Policies/DispatcherPolicy.hpp"
#include "Policies/StateChangeAdvertiserPolicy.hpp"
#include "Policies/SeekableOperations.hpp"
#include "Devices/DeviceFactory.hpp"
#include "Devices/DefaultDeviceDefinitions.h"
#include "Traits/device_traits.hpp"
#include "Traits/transport_traits.hpp"
#include "DeviceTypeSelector.hpp"
#include "Reactor/Reactor.hpp"
#include "Connector.hpp"
#include "ConfigurationBook.h"

namespace infra
{

/*
struct platform_traits
{
    using handle_t = int;
};
*/
using EpollReactor = Reactor<int, demux::EpollDemultiplexer<int>>;
using EpollConnector = Connector<EpollReactor>;
//DEFINE_STATE_CHANGE_ADVERTISER_POLICY(ConnectionState);

class DeviceTypeReader
{
public:
    virtual ~DeviceTypeReader() = default;

public:
    virtual std::size_t getDeviceType(const infra::config::ConfigurationBook & book,
                                     std::string_view section);
protected:
    infra::EDeviceType getSocketDeviceType(const infra::config::ConfigurationBook & book,
                                     std::string_view section);

    infra::EDeviceType getPipeDeviceType(const infra::config::ConfigurationBook & book,
                                     std::string_view section);
protected:
    using SockDomainToDevTypeMap = std::unordered_map<std::string, infra::EDeviceType>;
    using SockTypeMap = std::unordered_map<std::string, SockDomainToDevTypeMap>; 

    static SockTypeMap SocketConfigInfoToType;
};

template<template<typename...> typename EventHandlingPolicy,
         template<typename...> typename DispatcherPolicy,
         template<typename...> typename ConnectionStateChangeAdvertiserPolicy,
         template<typename...> typename... Ts>
struct generate_endpoint_typelist
{
    using type = meta::ttl::template_typelist<EventHandlingPolicy, DispatcherPolicy, ConnectionStateChangeAdvertiserPolicy, Ts...>;
};

struct default_client_traits
{
    // device traits
    using DeviceSet = default_device_set;
    using ResourceHandler = UnixResourceHandler;
    using DevicePolicies = meta::ttl::template_typelist<ConnectionPolicy, ResourceStatusPolicy, GenericIOPolicy>;
    using ExportPolicies = meta::ttl::template_typelist<ResourceStatusPolicy, SeekableOperations>;

    using TransportPolicies = meta::ttl::template_typelist</*ConnectionStateChangeAdvertiserPolicy*/>;
    using Listener = Reactor<int, demux::EpollDemultiplexer<int>>;
    using EventHandlingPolicy = meta::ttl::pack<BaseEventHandlingPolicy>;
    using DispatcherPolicy = meta::ttl::pack<BaseDispatcherPolicy>;
};

template<typename client_traits = default_client_traits>
class ConnectorClient
{
    using ResourceHandler = typename client_traits::ResourceHandler;
    //using TransportPolicies = typename client_traits::TransportPolicies;
    using DevicePolicies = typename client_traits::DevicePolicies;
    using Listener = typename client_traits::Listener;
    using ConcreteDeviceTypes = typename generate_device_typelist<ResourceHandler, DevicePolicies>::type;
    using StaticTransportEndpoint = typename transport_traits<unx_strm_tag, client_traits>::transport_endpoint_t;

public:
    ConnectorClient(Connector<Listener> & connector, std::string_view file_name):
        config_file{file_name},
        m_connector{connector}
    {}

public:
    void init(std::string_view section) {
        infra::config::ConfigurationBook book{config_file};
        if (!book.init()) return;
        m_device_type = static_cast<int>(DeviceTypeReader{}.getDeviceType(book, section));
        auto dynamic_completion_cb = [](auto endpoint) { (void) endpoint; std::cout << "here\n"; };
        auto static_completion_cb = [] (std::unique_ptr<ITransportEndpoint>&&) { std::cout <<"here 2\n"; };

        ConnectorAdapter::connect<client_traits>(m_device_type, book, section, m_connector, dynamic_completion_cb);
                                                 //[](std::unique_ptr<ITransportEndpoint> p) {});
        ConnectorAdapter::connect<client_traits, unx_strm_tag>(book, section, m_connector, static_completion_cb);
        /*
        DeviceTypeSelector<>::connect<ResourceHandler, DevicePolicies, TransportPolicies>
                                     (m_device_type, book, section, m_connector, callable);
        */
    }

    inline int getDeviceType() const { return m_device_type; }

private:
    std::string config_file;
    int m_device_type{static_cast<int>(infra::EDeviceType::E_UNDEFINED_DEVICE)};

    Connector<Listener> & m_connector;
    std::unique_ptr<StaticTransportEndpoint> m_p_static_transport_endpoint;
};

void toLower(std::string & value);

} // infra

#endif
