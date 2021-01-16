#ifndef DEVICE_TYPE_SELECTOR_HPP
#define DEVICE_TYPE_SELECTOR_HPP
#include "Devices/DeviceAddressFactory.hpp"
#include "ConfigurationBook.h"
#include "ConnectionParameters.hpp"

namespace infra
{

    template<typename Connector,
             typename ResourceHandler,
             typename DevicePolicies,
             typename TransportPolicies,
             typename CompletionCallback,
             std::size_t DeviceTag>
    bool connect(const config::ConfigurationBook & book, std::string_view section,
                 Connector & connector, CompletionCallback && cb) {
            using dev_addr_t = typename DeviceAddressFactory<DeviceTag>::DeviceAddressT; 
            ConnectionParameters<DeviceTag, dev_addr_t> conn_params;
            conn_params.address = DeviceAddressFactory<DeviceTag>::createAddress(book, section);
            return connector.template setup<ResourceHandler, DevicePolicies, TransportPolicies>
                                            (conn_params, std::forward<CompletionCallback>(cb));
    }

// delegate the selection of device address type and device type to this class
// based on non type tag
template<std::size_t device_tag, typename Enable = void>
class DeviceTypeSelector
{
    template<typename Connector,
             typename ResourceHandler,
             typename DevicePolicies,
             typename TransportPolicies,
             typename CompletionCallback>
    static bool connect( const infra::config::ConfigurationBook & book, const std::string & section,
                         Connector & connector, CompletionCallback && cb)
    {
        bool ret{false};
        switch (device_tag)
        {
        case ipv4_strm_tag:
        {
            ret = connect<Connector, ResourceHandler, DevicePolicies, TransportPolicies, CompletionCallback, ipv4_strm_tag>
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

}//infra
    
#endif
