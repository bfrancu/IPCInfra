#ifndef CONNECTOR_CLIENT_H
#define CONNECTOR_CLIENT_H
#include <string>
#include <unordered_map>

#include "Host.hpp"
#include "typelist.hpp"
#include "template_typelist.hpp"
#include "Devices/DeviceDefinitions.h"
#include "Policies/ResourceStatusPolicy.hpp"
#include "Devices/DeviceFactory.hpp"
#include "TransportEndpoint.hpp"
#include "ConfigurationBook.h"

namespace infra
{

/*
struct platform_traits
{
    using handle_t = int;
};
*/

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


struct default_client_traits
{
    using ResourceHandler = UnixResourceHandler;
    using DevicePolicies = meta::ttl::template_typelist<ResourceStatusPolicy>; 
    using TransportPolicies = meta::ttl::template_typelist<>;
};

template<typename client_traits = default_client_traits>
class ConnectorClient
{
    using ResourceHandler = typename client_traits::ResourceHandler;
    using TransportPolicies = typename client_traits::TransportPolicies;
    using DevicePolicies = typename client_traits::DevicePolicies;

    using ConcreteDeviceTypes = typename generate_device_typelist<ResourceHandler, DevicePolicies>::type;

public:
    ConnectorClient(std::string_view file_name):
        config_file{file_name},
        m_pEndpoint{nullptr}
    {}

public:
    void init(std::string_view section) {
        infra::config::ConfigurationBook book{config_file};
        if (!book.init()) return;
        m_device_type = static_cast<int>(DeviceTypeReader{}.getDeviceType(book, section));
    }

    inline int getDeviceType() const { return m_device_type; }

private:
    std::string config_file;
    int m_device_type{static_cast<int>(infra::EDeviceType::E_UNDEFINED_DEVICE)};
    std::unique_ptr<AbstractTransportEndpoint> m_pEndpoint;
};

void toLower(std::string & value);

} // infra

#endif
