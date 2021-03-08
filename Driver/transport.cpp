#include "Devices/Sockets/UnixSocketAddress.h"
#include "Traits/device_constraints.hpp"
#include "Devices/Sockets/SocketDevice.hpp"
#include "Devices/Pipes/NamedPipeDevice.hpp"
#include "Devices/Pipes/NamedPipeAddress.h"
#include "Devices/FileDevice.hpp"
#include "Devices/DeviceAddressFactory.hpp"
#include "Devices/DefaultDeviceDefinitions.h"
#include "Policies/UnixResourceHandler.h"
#include "Devices/DeviceDefinitions.h"
#include "Policies/FifoIOPolicy.hpp"
#include "Policies/ConnectionPolicy.hpp"
#include "Policies/ResourceStatusPolicy.hpp"
#include "Host.hpp"
#include "template_typelist.hpp"
#include <type_traits>

namespace infra
{

namespace transport
{

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
struct test_is_unix_socket_device : std::false_type {};

template<typename T>
struct test_is_unix_socket_device<T, std::void_t<traits::UnixSocketDevice<T>>> : std::true_type {};

static_assert(test_is_unix_socket_device<defaults::UnixStreamSocketDevice>::value);
static_assert(test_is_unix_socket_device<defaults::UnixDgramSocketDevice>::value);
static_assert(test_is_unix_socket_device<defaults::IPV4TcpSocketDevice>::value);
static_assert(!test_is_unix_socket_device<defaults::ReadFifoDevice>::value);

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

namespace policies
{
//DEFINE_HAS_MEMBER(read);
//DEFINE_HAS_MEMBER(open);
//DEFINE_HAS_TYPE(io_profile);
DEFINE_HAS_MEMBER(isReadable);

using ReadFifoWithPolicies = PackHostT<defaults::ReadFifoDevice, 
                                       meta::ttl::template_typelist<FifoIOPolicy,
                                                                    ResourceStatusPolicy>>;

static_assert(has_member_isReadable<ReadFifoWithPolicies>::value);

using FileWithPoliies = PackHostT<defaults::UnixFileDevice, 
                                  meta::ttl::template_typelist<FifoIOPolicy,
                                  ResourceStatusPolicy>>;

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
                         traits::UnixSocketDevice<Device>
                         //Device,
                         //std::void_t<traits::UnixSocketDevice<Device>>
                         //std::void_t<std::enable_if_t<IsUnixSocketDeviceT<Device>::value>>
                         //std::enable_if_t<IsUnixSocketDeviceT<Device>::value>
                         >
      : public crtp_base<TestPolicy<Host, Device>, Host>
{

    using test_tag = unix_socket_device_test_tag;
};

template<typename Host, typename Device>
struct TestPolicy<Host,
                       traits::NamedPipeDevice<Device>
                       //std::enable_if_t<UnixNamedPipeDevice<Device>::value && std::negation_v<IsUnixSocketDeviceT<Device>>>
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

}
}//transport
}//infra
