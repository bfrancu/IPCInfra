#ifndef ACCEPTORCLIENT_H
#define ACCEPTORCLIENT_H
#include "Acceptor.hpp"

#include "non_typelist.hpp"

#include "Policies/EndpointStorage.hpp"
#include "Policies/ServerCallbackPolicy.hpp"
#include "Policies/AcceptorPolicy.hpp"
#include "Policies/UnixResourceHandler.h"
#include "Devices/DeviceFactory.hpp"
#include "Devices/DefaultDeviceDefinitions.h"
#include "Traits/transport_traits.hpp"

#include "ConnectorClient.h"

namespace infra
{

template <std::size_t... device_tags>
struct tag_holder {};

using t = typename generate_endpoint_typelist<default_client_traits, tag_holder<ipv4_strm_tag>>::type;
static_assert (std::is_same_v<t, meta::tl::typelist<typename transport_traits<ipv4_strm_tag,
                                                                              default_client_traits>::transport_endpoint_t>>);

struct default_server_traits
{
    using PeerTraits = default_client_traits;

    //using DeviceSet = meta::ntl::non_typelist<ipv4_strm_tag, read_fifo_tag>;
    using DeviceSet = default_device_set;
    using ResourceHandler = UnixResourceHandler;
    using DevicePolicies = meta::ttl::template_typelist<AcceptorPolicy, GenericIOPolicy>;
    using ExportPolicies = meta::ttl::template_typelist<>;

    using TransportPolicies = meta::ttl::template_typelist<>;
    using Listener = Reactor<int, demux::EpollDemultiplexer<int>>;
    using ClientServerRolePolicy = meta::ttl::pack<ServerCallbackPolicy>;
    using EndpointStorageKey = typename ResourceHandler::handle_type;
    using EndpointStoragePolicy = meta::ttl::pack<AssociativeEndpointStorage>;
};

template<typename server_traits = default_server_traits>
class AcceptorClient
{
    static constexpr std::size_t static_device_tag{read_fifo_tag};
    using ResourceHandler = typename server_traits::ResourceHandler;
    using DevicePolicies = typename server_traits::DevicePolicies;
    using Listener = typename server_traits::Listener;
    using EndpointTypes = typename generate_endpoint_typelist<server_traits>::type;
    using ClientKey = typename server_traits::EndpointStorageKey;
    using StaticServerTransportEndpoint = typename transport_traits<static_device_tag, server_traits>::transport_endpoint_t;

public:
    AcceptorClient(Acceptor<Listener> & acceptor, std::string_view file_name) :
        config_file{file_name},
        m_acceptor{acceptor},
        m_p_dynamic_transport_endpoint{std::make_unique<ServerDynamicTransportEndpointAdapter<EndpointTypes>>()}
    {}

public:
    void init(std::string_view section) 
    {
        infra::config::ConfigurationBook book{config_file};
        if (!book.init()) return;
        m_device_type = static_cast<int>(DeviceTypeReader{}.getDeviceType(book, section));
        if (static_cast<int>(EDeviceType::E_UNDEFINED_DEVICE) == m_device_type)
        {
            std::cerr << "Invalid device type\n";
            return;
        }
        else
        {
            std::cout << "Static device type is: " << static_device_tag << "; dynamic device type is: " << m_device_type << "\n";
        }

        auto dynamic_completion_cb = [this](std::unique_ptr<IServerTransportEndpoint> && p_endpoint){
            std::cout << "dynamic endpoint cb\n";
            if (!p_endpoint) {
                std::cout << "Empty dynamic endpoint\n";
                return;
            }

            if (!m_p_dynamic_transport_endpoint->init(std::move(p_endpoint), m_device_type))
            {
                std::cout << "dynamic transport endpoint initialization failed\n";
                return;
            }

            m_p_dynamic_transport_endpoint->registerClientInputCallback([this](const ClientKey & key, std::string_view content){
                        std::cout << "[dynamic] received from client key: " << key << "; content: " << content << "\n";
                        m_p_dynamic_transport_endpoint->send(key, "roger roger\n");
                    });

            m_p_dynamic_transport_endpoint->registerClientConnectionCallback([](const ClientKey & key){
                        std::cout << "[dynamic] client with key: " << key << " is now connected\n";
                    });

            m_p_dynamic_transport_endpoint->registerClientDisconnectionCallback([](const ClientKey & key){
                        std::cout << "[dynamic] client with key: " << key << " is now disconnected\n";
                    });
        };

        auto static_completion_cb = [this](std::unique_ptr<IServerTransportEndpoint> && p_endpoint){
            std::cout <<"static endpoint cb\n";
            if (!p_endpoint) {
                std::cout << "Empty static endpoint\n";
                return;
            }

            m_p_static_transport_endpoint.reset(reinterpret_cast<StaticServerTransportEndpoint*>(p_endpoint->releaseInternalEndpoint()));
            m_p_static_transport_endpoint->registerClientInputCallback([](const ClientKey & key, std::string_view content){
                        std::cout << "[static] received from client key: " << key << "; content: " << content << "\n";
                    });

            m_p_static_transport_endpoint->registerClientConnectionCallback([](const ClientKey & key){
                        std::cout << "[static] client with key: " << key << " is now connected\n";
                    });

            m_p_static_transport_endpoint->registerClientDisconnectionCallback([](const ClientKey & key){
                        std::cout << "[static] client with key: " << key << " is now disconnected\n";
                    });
        };

        //std::cout << "\nSetting up dynamic server endpoint\n";
        //ConnectionInitializerAdapter::connect<server_traits>(m_device_type, book, section, m_acceptor, dynamic_completion_cb);
        std::cout << "\nSetting up static server endpoint\n";
        ConnectionInitializerAdapter::connect<server_traits, static_device_tag>(book, section, m_acceptor, static_completion_cb);
    }

private:
    std::string config_file;
    int m_device_type{static_cast<int>(infra::EDeviceType::E_UNDEFINED_DEVICE)};

    Acceptor<Listener> & m_acceptor;
    std::unique_ptr<StaticServerTransportEndpoint> m_p_static_transport_endpoint;
    std::unique_ptr<ServerDynamicTransportEndpointAdapter<EndpointTypes>> m_p_dynamic_transport_endpoint;
};

}//infra

#endif // ACCEPTORCLIENT_H
