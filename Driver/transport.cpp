#include "transport.h"

#include <iostream>
#include <type_traits>

#include "Devices/Sockets/UnixSocketAddress.h"
#include "Reactor/EpollDemultiplexer.h"
#include "Traits/device_constraints.hpp"
#include "Traits/transport_traits.hpp"
#include "Traits/utilities_traits.hpp"
#include "Traits/storage_traits.hpp"
#include "Devices/Sockets/SocketDevice.hpp"
#include "Devices/Pipes/NamedPipeDevice.hpp"
#include "Devices/Pipes/NamedPipeAddress.h"
#include "Devices/FileDevice.hpp"
#include "Devices/DeviceAddressFactory.hpp"
#include "Devices/DefaultDeviceDefinitions.h"
#include "Devices/ProxyDevice.hpp"
#include "Policies/UnixResourceHandler.h"
#include "Policies/ClientCallbackPolicy.hpp"
#include "Devices/DeviceDefinitions.h"
#include "Policies/FifoIOPolicy.hpp"
#include "Policies/ConnectionPolicy.hpp"
#include "Policies/ResourceStatusPolicy.hpp"
#include "Policies/IOPolicy.hpp"
#include "Policies/SeekableOperations.hpp"
#include "Policies/AcceptorPolicy.hpp"
#include "Policies/ExporterPolicy.hpp"
#include "Reactor/Reactor.hpp"
#include "Reactor/DeviceTestEventHandler.h"
#include "ConnectorClient.h"
#include "AcceptorClient.h"
#include "Host.hpp"
#include "TransportDefinitions.h"
#include "template_typelist.hpp"
#include "utilities.hpp"
#include "Policies/EndpointStorage.hpp"

namespace infra
{

namespace transport
{

template<typename Host, typename Device>
class CustomFacadePolicy : public ResourceStatusPolicy<Host, Device>,
                     public SeekableOperations<Host, Device>
{
};

template<typename Host, typename Device, typename TList>
class PolicyHierarchy;

template<typename Host,
         typename Device,
         template <typename...> typename... Policies>
class PolicyHierarchy<Host, 
                      Device,
                      meta::ttl::template_typelist<Policies...>> : public Policies<Host, Device>...
{};

template<typename Dev>
struct DeviceHolder
{
    using Device = Dev;
    using handle_type = typename Device::handle_type;
    using platform = unix_platform;

    DeviceHolder(Device & device):
        m_device(device)
    {}

    Device & getDevice() { return m_device; }
    //handle_type getHandle() { return m_device.getHandle(); }

    Device & m_device;
};

//template<template <typename...> typename... Policis>

void testConnectorClient()
{
    using namespace infra;
    using handleT = int;
    using ReactorT = Reactor<handleT, demux::EpollDemultiplexer<handleT>>;
    ReactorT reactor;
    connect_R<ReactorT> connector(reactor);
    //reactor.start();
    //reactor.stop();
    std::string config_file{"/home/bfrancu/Documents/Work/Projects/IPCInfra/Configuration/example.ini"};
    std::string config_section{"CONNECTION_DETAILS"};

    reactor.start();
    ConnectorClient<default_client_traits> client{connector, config_file};
    client.init(config_section);
    for (;;) {}
}

void testAcceptorClient()
{
    using namespace infra;
    using handleT = int;
    using ReactorT = Reactor<handleT, demux::EpollDemultiplexer<handleT>>;
    ReactorT reactor;
    Acceptor<ReactorT> acceptor(reactor);
    std::string config_file{"/home/bfrancu/Documents/Work/Projects/IPCInfra/Configuration/example.ini"};
    std::string config_section{"CONNECTION_DETAILS"};
    AcceptorClient<default_server_traits> server{acceptor, config_file};
    /*
    constexpr auto dev_tag = write_fifo_tag;
    using ServerEndpointStorageT = typename transport_traits<dev_tag, default_server_traits>::endpoint_storage_t;
    using DeviceT = typename transport_traits<dev_tag, default_server_traits>::device_host_t;
    using EndpointT = typename transport_traits<dev_tag, default_server_traits>::transport_endpoint_t;
    using DeviceAddressT = typename transport_traits<dev_tag, default_server_traits>::device_address_t;
    //using UnixSocketWithPolicies = PackHostT<defaults::ReadFifoDevice, meta::ttl::template_typelist<AcceptorPolicy>>;
    EndpointT endpoint(reactor);
    static_assert(IsPassiveConnectableDevice<DeviceT>::value);
    static_assert (traits::is_endpoint_storage_v<ServerEndpointStorageT>);
    DeviceT dev;
    dev.bind(DeviceAddressT{}, true);
    */

    reactor.start();
    server.init(config_section);
    std::cout << "you are here\n";
    for (;;) {}
}

DEFINE_HAS_MEMBER(init);

void transport_main()
{
    defaults::UnixFileDevice file_dev;
    defaults::UnixDgramSocketDevice unx_dgram_dev;
    defaults::IPV6TcpSocketDevice ipv6_strm_dev;
    ProxyDevice<defaults::UnixFileDevice> proxy_dev;
    proxy_dev.setBaseReference(file_dev);
    //auto file_handle = file_dev.getHandle();
    auto proxy_handle = proxy_dev.getHandle();

    //std::cout << "transport_main() before open, file_handle " << file_handle << " proxy_handle " << proxy_handle << "\n";

    //file_dev.open("/home/bfrancu/Documents/Work/Projects/IPCInfra/TransportInfra/Devices/TestDevice.h", io::EAccessMode::E_READ_ONLY);
    //file_dev.close();

    //std::cout << "transport_main() after open file_handle " << file_handle << " proxy_handle " << proxy_handle << "\n";
    using assembled_proxy_t = Host<ProxyDevice<defaults::UnixFileDevice>,
                                      ResourceStatusPolicy, SeekableOperations>;

    assembled_proxy_t proxy_host;
    proxy_host.setBaseReference(file_dev);

    using assembled_proxy_t2 = Host<ProxyDevice<defaults::UnixDgramSocketDevice>, CustomFacadePolicy>;

    assembled_proxy_t2 proxy_host2;
    proxy_host2.setBaseReference(unx_dgram_dev);
    std::cout << "is tty: " << proxy_host2.isTty() << "\n";
    std::cout << " file type: " << utils::to_underlying(proxy_host2.fileType()) << "\n";
    std::cout << " file type: " << utils::to_underlying(proxy_host.fileType()) << "\n";

    using assembled_proxy_t3 = Host<ProxyDevice<defaults::IPV6TcpSocketDevice>,
                                    generate_hierarchy<meta::ttl::template_typelist<ResourceStatusPolicy,
                                                                                    SeekableOperations>>::policy_type>;

    assembled_proxy_t3 proxy_host3;
    proxy_host3.setBaseReference(ipv6_strm_dev);
    std::cout << " file type: " << utils::to_underlying(proxy_host3.fileType()) << "\n";

    using UnixSocketWithPolicies = PackHostT<defaults::UnixStreamSocketDevice, meta::ttl::template_typelist<AcceptorPolicy>>;
    UnixSocketWithPolicies s{};

    using exported_policies_t = meta::ttl::template_typelist<ResourceStatusPolicy, SeekableOperations>;
    using reactor_t = Reactor<int, demux::EpollDemultiplexer<int>>;
    using device_policies_t = exported_policies_t;
    using file_device_t = PackHostT<defaults::UnixFileDevice, device_policies_t>;
    //using dev_holder_t = DeviceTestEventHandler<file_device_t, reactor_t>;
    using dev_holder_t = DeviceHolder<file_device_t>;

    //using assembled_dev_holder_t1 = PackHostT<dev_holder_t, meta::ttl::template_typelist<Exporter<exported_policies_t>::template Policy>>;
    //using assembled_dev_holder_t1 = PackHostT<defaults::UnixFileDevice, meta::ttl::template_typelist<Exporter<exported_policies_t>::template Policy>>;
    using assembled_dev_holder_t1 = Host<dev_holder_t, Exporter<exported_policies_t>::template Policy, GenericIOPolicy>;
    //using exported_policy = Exporter<exported_policies_t>::template Policy<
    //reactor_t reactor;
    file_device_t file_dev_host;
    assembled_dev_holder_t1 dev_t1(file_dev_host);
    std::cout << "Initialising device...\n\n\n";
    dev_t1.init();
    if(!file_dev_host.open("/home/bfrancu/Documents/Work/Projects/IPCInfra/TransportInfra/Devices/TestDevice.h", io::EAccessMode::E_READ_ONLY))
    {
        std::cerr << "file open fail with errno " << errno << "\n";
    }
    std::cout << "Exporter file device; is readable: " << dev_t1.isReadable() << "\n";
    std::string content;
    /*
    if (0 < dev_t1.read(content))
    {
        std::cout << content << "\n";
    }
    else
    {
        std::cout << "problem reading\n";
    }
    */
}

static_assert(std::is_same_v<typename ProxyDevice<defaults::UnixFileDevice>::handle_type, 
        typename defaults::UnixFileDevice::handle_type>);

static_assert(std::is_same_v<typename ProxyDevice<defaults::UnixFileDevice>::platform, 
        typename defaults::UnixFileDevice::platform>);

static_assert(std::is_same_v<typename ProxyDevice<defaults::UnixFileDevice>::sequential, 
        typename defaults::UnixFileDevice::sequential>);

static_assert(std::is_same_v<typename ProxyDevice<defaults::IPV4TcpSocketDevice>::socket_domain, 
        typename defaults::IPV4TcpSocketDevice::socket_domain>);

static_assert(std::is_same_v<typename ProxyDevice<defaults::IPV6TcpSocketDevice>::address_type, 
        typename defaults::IPV6TcpSocketDevice::address_type>);

static_assert(std::is_same_v<typename ProxyDevice<defaults::IPV4UdpSocketDevice>::socket_type, 
        typename defaults::IPV4UdpSocketDevice::socket_type>);

static_assert(std::is_same_v<typename ProxyDevice<defaults::IPV4UdpSocketDevice>::socket_type, 
        typename defaults::IPV4UdpSocketDevice::socket_type>);

static_assert(std::is_same_v<typename ProxyDevice<defaults::UnixStreamSocketDevice>::socket_type, 
        typename defaults::UnixStreamSocketDevice::socket_type>);

static_assert(std::is_same_v<typename ProxyDevice<defaults::ReadFifoDevice>::io_profile, 
        typename defaults::ReadFifoDevice::io_profile>);

static_assert(std::is_same_v<typename ProxyDevice<defaults::WriteFifoDevice>::address_type, 
        typename defaults::WriteFifoDevice::address_type>);

template<typename T, typename = void>
struct test_is_unix_device : std::false_type {};

template<typename T>
struct test_is_unix_device<T, std::void_t<traits::UnixDevice<T>>> : std::true_type {};

static_assert(test_is_unix_device<defaults::IPV4TcpSocketDevice>::value);
static_assert(test_is_unix_device<defaults::IPV6UdpSocketDevice>::value);
static_assert(test_is_unix_device<defaults::UnixStreamSocketDevice>::value);
static_assert(test_is_unix_device<defaults::WriteFifoDevice>::value);
static_assert(test_is_unix_device<defaults::UnixFileDevice>::value);

template<typename T, typename = void>
struct test_is_socket_device : std::false_type {};

template<typename T>
struct test_is_socket_device<T, std::void_t<traits::SocketDevice<T>>> : std::true_type {};

static_assert(test_is_socket_device<defaults::IPV4UdpSocketDevice>::value);
static_assert(test_is_socket_device<defaults::IPV6TcpSocketDevice>::value);
static_assert(test_is_socket_device<defaults::UnixDgramSocketDevice>::value);
static_assert(!test_is_socket_device<defaults::WriteFifoDevice>::value);
static_assert(!test_is_socket_device<defaults::UnixFileDevice>::value);

template<typename T, typename = void>
struct test_is_stream_socket_device : std::false_type {};

template<typename T>
struct test_is_stream_socket_device<T, std::void_t<traits::StreamSocketDevice<T>>> : std::true_type {};

static_assert(!test_is_stream_socket_device<defaults::IPV4UdpSocketDevice>::value);
static_assert(!test_is_stream_socket_device<defaults::IPV6UdpSocketDevice>::value);
static_assert(!test_is_stream_socket_device<defaults::UnixDgramSocketDevice>::value);
static_assert(test_is_stream_socket_device<defaults::IPV4TcpSocketDevice>::value);
static_assert(test_is_stream_socket_device<defaults::IPV6TcpSocketDevice>::value);
static_assert(test_is_stream_socket_device<defaults::UnixStreamSocketDevice>::value);
static_assert(!test_is_stream_socket_device<defaults::ReadFifoDevice>::value);
static_assert(!test_is_stream_socket_device<defaults::WriteFifoDevice>::value);
static_assert(!test_is_stream_socket_device<defaults::UnixFileDevice>::value);

template<typename T, typename = void>
struct test_is_datagram_socket_device : std::false_type {};

template<typename T>
struct test_is_datagram_socket_device<T, std::void_t<traits::DatagramSocketDevice<T>>> : std::true_type {};

static_assert(test_is_datagram_socket_device<defaults::IPV4UdpSocketDevice>::value);
static_assert(test_is_datagram_socket_device<defaults::IPV6UdpSocketDevice>::value);
static_assert(test_is_datagram_socket_device<defaults::UnixDgramSocketDevice>::value);
static_assert(!test_is_datagram_socket_device<defaults::IPV4TcpSocketDevice>::value);
static_assert(!test_is_datagram_socket_device<defaults::IPV6TcpSocketDevice>::value);
static_assert(!test_is_datagram_socket_device<defaults::UnixStreamSocketDevice>::value);
static_assert(!test_is_datagram_socket_device<defaults::ReadFifoDevice>::value);
static_assert(!test_is_datagram_socket_device<defaults::WriteFifoDevice>::value);
static_assert(!test_is_datagram_socket_device<defaults::UnixFileDevice>::value);

template<typename T, typename = void>
struct test_is_unix_platform_socket_device : std::false_type {};

template<typename T>
struct test_is_unix_platform_socket_device<T, std::void_t<traits::UnixPlatformSocketDevice<T>>> : std::true_type {};

static_assert(test_is_unix_platform_socket_device<defaults::UnixStreamSocketDevice>::value);
static_assert(test_is_unix_platform_socket_device<defaults::UnixDgramSocketDevice>::value);
static_assert(test_is_unix_platform_socket_device<defaults::IPV4TcpSocketDevice>::value);
static_assert(!test_is_unix_platform_socket_device<defaults::ReadFifoDevice>::value);

template<typename T, typename = void>
struct test_is_unix_socket_device : std::false_type {};

template<typename T>
struct test_is_unix_socket_device<T, std::void_t<traits::UnixSocketDevice<T>>> : std::true_type {};

static_assert(test_is_unix_socket_device<defaults::UnixStreamSocketDevice>::value);
static_assert(!test_is_unix_socket_device<defaults::IPV6UdpSocketDevice>::value);

static_assert(UnixSocketRequires<UnixResourceHandler, unx::UnixSocketAddress>::value);
static_assert(!UnixSocketRequires<UnixResourceHandler, IPV4InetSocketAddress>::value);
static_assert(!UnixSocketRequires<UnixResourceHandler, IPV6InetSocketAddress>::value);
static_assert(!UnixSocketRequires<UnixResourceHandler, NamedPipeAddress>::value);

static_assert(!IPV4SocketRequires<UnixResourceHandler, unx::UnixSocketAddress>::value);
static_assert(IPV4SocketRequires<UnixResourceHandler, IPV4InetSocketAddress>::value);
static_assert(!IPV4SocketRequires<UnixResourceHandler, IPV6InetSocketAddress>::value);
static_assert(!IPV4SocketRequires<UnixResourceHandler, NamedPipeAddress>::value);

static_assert(!IPV6SocketRequires<UnixResourceHandler, unx::UnixSocketAddress>::value);
static_assert(!IPV6SocketRequires<UnixResourceHandler, IPV4InetSocketAddress>::value);
static_assert(IPV6SocketRequires<UnixResourceHandler, IPV6InetSocketAddress>::value);
static_assert(!IPV6SocketRequires<UnixResourceHandler, NamedPipeAddress>::value);

template<typename T, typename = void>
struct test_is_named_pipe_device : std::false_type {};

template<typename T>
struct test_is_named_pipe_device<T, std::void_t<traits::NamedPipeDevice<T>>> : std::true_type {};

static_assert(test_is_named_pipe_device<defaults::WriteFifoDevice>::value);
static_assert(test_is_named_pipe_device<defaults::ReadFifoDevice>::value);
static_assert(!test_is_named_pipe_device<defaults::IPV4TcpSocketDevice>::value);
static_assert(!test_is_named_pipe_device<defaults::IPV6TcpSocketDevice>::value);
static_assert(!test_is_named_pipe_device<defaults::IPV6UdpSocketDevice>::value);
static_assert(!test_is_named_pipe_device<defaults::IPV4UdpSocketDevice>::value);
static_assert(!test_is_named_pipe_device<defaults::UnixStreamSocketDevice>::value);
static_assert(!test_is_named_pipe_device<defaults::UnixDgramSocketDevice>::value);

template<typename T, typename = void>
struct test_is_unix_named_pipe_device : std::false_type {};

template<typename T>
struct test_is_unix_named_pipe_device<T, std::void_t<traits::UnixNamedPipeDevice<T>>> : std::true_type {};

static_assert(std::is_same_v<int, typename defaults::WriteFifoDevice::handle_type>);
static_assert(test_is_unix_named_pipe_device<defaults::WriteFifoDevice>::value);
static_assert(test_is_unix_named_pipe_device<defaults::ReadFifoDevice>::value);
static_assert(!test_is_unix_named_pipe_device<defaults::IPV4TcpSocketDevice>::value);
static_assert(!test_is_unix_named_pipe_device<defaults::IPV6TcpSocketDevice>::value);
static_assert(!test_is_unix_named_pipe_device<defaults::IPV6UdpSocketDevice>::value);
static_assert(!test_is_unix_named_pipe_device<defaults::IPV4UdpSocketDevice>::value);
static_assert(!test_is_unix_named_pipe_device<defaults::UnixStreamSocketDevice>::value);
static_assert(!test_is_unix_named_pipe_device<defaults::UnixDgramSocketDevice>::value);

template<typename T, typename = void>
struct test_is_fifo_device : std::false_type {};

template<typename T>
struct test_is_fifo_device<T, std::void_t<traits::FifoDevice<T>>> : std::true_type {};

static_assert(test_is_fifo_device<defaults::WriteFifoDevice>::value);
static_assert(test_is_fifo_device<defaults::ReadFifoDevice>::value);

DEFINE_HAS_TYPE(sequential);
DEFINE_HAS_TYPE(socket_domain);
DEFINE_HAS_TYPE(io_profile);
using test_socket_traits = traits::select_traits<defaults::IPV4TcpSocketDevice>;
using test_fifo_traits = traits::select_traits<defaults::WriteFifoDevice>;
using test_device_traits = traits::select_traits<defaults::UnixFileDevice>;

static_assert(has_type_io_profile<test_fifo_traits>::value);
static_assert(has_type_socket_domain<test_socket_traits>::value);
static_assert(has_type_sequential<test_device_traits>::value);
static_assert(!has_type_io_profile<test_socket_traits>::value);

DEFINE_MEMBER_TYPE_OR_DEFAULT(socket_type);
static_assert(std::is_same_v<socket_type_or_default_t<defaults::IPV4TcpSocketDevice, int>, stream_socket>);

namespace policies
{
//DEFINE_HAS_MEMBER(read);
//DEFINE_HAS_MEMBER(open);
DEFINE_HAS_MEMBER(isReadable);

using ReadFifoWithPolicies = PackHostT<defaults::ReadFifoDevice, 
                                       meta::ttl::template_typelist<FifoIOPolicy,
                                                                    ResourceStatusPolicy>>;

static_assert(has_member_isReadable<ReadFifoWithPolicies>::value);

using FileWithPolicies = PackHostT<defaults::UnixFileDevice, 
                                  meta::ttl::template_typelist<FifoIOPolicy,
                                  ResourceStatusPolicy>>;


using AcceptorSocket = PackHostT<defaults::IPV6TcpSocketDevice,
                                 meta::ttl::template_typelist<AcceptorPolicy>>;

static_assert(IsPassiveConnectableDevice<AcceptorSocket>::value);
static_assert(!IsPassiveConnectableDevice<FileWithPolicies>::value);
static_assert(traits::apply_predicates_v<AcceptorSocket, def::has_member_bind, def::has_member_listen, def::has_member_accept>);
static_assert(!traits::apply_predicates_v<AcceptorSocket, def::has_member_connect, def::has_member_listen, def::has_member_accept>);
//static_assert(std::is_same_v<int, typename UnixSocketWithPolicies::test_type>);
//make an empty policy class that is partial specialized by traits
//each specialization exposes a different type
//assert that type to test the specialization by trait

template<typename Host, typename Device, typename = void>
struct TestPolicy{};

struct unix_socket_device_test_tag{};
struct stream_socket_device_test_tag{};
struct unix_named_pipe_device_test_tag{};
struct dgram_socket_device_test_tag{};

template<typename Host, typename Device>
struct TestPolicy<Host, 
                         traits::UnixPlatformSocketDevice<Device>
                         //Device,
                         //std::void_t<traits::UnixPlatformSocketDevice<Device>>
                         //std::void_t<std::enable_if_t<IsUnixPlatformSocketDeviceT<Device>::value>>
                         //std::enable_if_t<IsUnixPlatformSocketDeviceT<Device>::value>
                         >
      : public crtp_base<TestPolicy<Host, Device>, Host>
{

    using test_tag = unix_socket_device_test_tag;
};

template<typename Host, typename Device>
struct TestPolicy<Host,
                       //traits::NamedPipeDevice<Device>
                       Device, std::enable_if_t<UnixNamedPipeDevice<Device>::value && std::negation_v<IsUnixPlatformSocketDeviceT<Device>>>
                      > 
      : public crtp_base<TestPolicy<Host, Device>, Host>
{
    using test_tag = unix_named_pipe_device_test_tag;
};


using test_policies_pack = meta::ttl::template_typelist<TestPolicy, ConnectionPolicy>;

static_assert(std::is_same_v<typename PackHostT<defaults::UnixStreamSocketDevice,
                                                test_policies_pack>::test_tag, unix_socket_device_test_tag>);
                 
static_assert(std::is_same_v<typename PackHostT<defaults::ReadFifoDevice,
                                                test_policies_pack>::test_tag, unix_named_pipe_device_test_tag>);

static_assert(std::is_same_v<typename PackHostT<defaults::UnixStreamSocketDevice,
                                                test_policies_pack>::address_type, unx::UnixSocketAddress>);
                 
static_assert(std::is_same_v<typename PackHostT<defaults::WriteFifoDevice,
                                                test_policies_pack>::address_type, NamedPipeAddress>);

static_assert(std::is_same_v<typename DeviceAddressFactory<ipv4_strm_tag>::DeviceAddressT, IPV4InetSocketAddress>);
static_assert(std::is_same_v<typename DeviceAddressFactory<ipv6_dgram_tag>::DeviceAddressT, IPV6InetSocketAddress>);
static_assert(std::is_same_v<typename DeviceAddressFactory<unx_strm_tag>::DeviceAddressT, unx::UnixSocketAddress>);
static_assert(std::is_same_v<typename DeviceAddressFactory<read_fifo_tag>::DeviceAddressT, NamedPipeAddress>);


DEFINE_HAS_TYPE(Device);
DEFINE_HAS_MEMBER(connect);
DEFINE_HAS_MEMBER(disconnect);
DEFINE_HAS_MEMBER(fail);

using UnxStrmClientTransportEndpoint = typename transport_traits<unx_strm_tag, default_client_traits>::transport_endpoint_t;
static_assert(has_type_Device<UnxStrmClientTransportEndpoint>::value);

using UnxStrmClientDevice = typename UnxStrmClientTransportEndpoint::Device;

static_assert(has_member_connect<UnxStrmClientDevice>::value);
static_assert(has_member_disconnect<UnxStrmClientDevice>::value);

static_assert(HasConnectableDevice<UnxStrmClientTransportEndpoint>::value);

/*
struct TestClientCallbackPolicy : public ClientCallbackPolicy<TestClientCallbackPolicy, defaults::IPV4TcpSocketDevice>
{
    using Device = defaults::IPV4TcpSocketDevice;
    using addr_t = typename Device::address_type;
};
*/

//using TestClientCallbackPolicyDevice = typename TestClientCallbackPolicy::Device;
//static_assert(has_type_Device<TestClientCallbackPolicy>::value);
//static_assert (HasConnectableDevice<TestClientCallbackPolicy>::value);

static_assert(has_member_connect<UnxStrmClientTransportEndpoint>::value);

using SampleSingleStorage = SingleEndpointStorage<int, double>;
static_assert(traits::is_endpoint_storage_v<SampleSingleStorage>);
static_assert(def::has_member_store<SampleSingleStorage>::value);
static_assert(def::has_member_erase<SampleSingleStorage>::value);
static_assert(traits::has_member_getEndpointFor<SampleSingleStorage>::value);
static_assert(def::has_member_store<SampleSingleStorage>::value);

using ipv4_stream_socket_endpoint_t = typename transport_traits<ipv4_strm_tag, default_client_traits>::transport_endpoint_t;
using ipv4_stream_socket_endpoint_t_host_device = typename ipv4_stream_socket_endpoint_t::Device;
static_assert (std::is_same_v<typename UnpackHost<ipv4_stream_socket_endpoint_t_host_device>::ClientT, defaults::IPV4TcpSocketDevice>);
static_assert (Type2DeviceTag<defaults::IPV4TcpSocketDevice>::value == ipv4_strm_tag);
static_assert (type_to_device_tag<ipv4_stream_socket_endpoint_t>::value == ipv4_strm_tag);
static_assert (def::has_type_Device<ipv4_stream_socket_endpoint_t>::value);
static_assert (type_to_device_tag<ipv4_stream_socket_endpoint_t>::value == ipv4_strm_tag);

}
}//transport
}//infra
