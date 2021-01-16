#include "fcntl.h"
#include <sys/stat.h>
#include "Policies/UnixResourceHandler.h"
#include "Host.hpp"
#include "TypeList.hpp"
#include "Devices/FileDevice.hpp"
#include "Policies/SeekableOperations.hpp"
#include "Policies/ResourceStatusPolicy.hpp"
#include "Policies/IOPolicy.hpp"
#include "utilities.hpp"
#include "LinuxUtils/LinuxIOUtilities.h"
#include "Devices/TemporaryFileDevice.hpp"
#include "Devices/Sockets/SocketDevice.hpp"
#include "Policies/ConnectionPolicy.hpp"
#include "Policies/AcceptorPolicy.hpp"
//#include "Policies/ErrorChangeAdvertiserPolicy.h"
#include "Devices/Sockets/UnixSocketAddress.h"
#include "Devices/Sockets/HostAddress.hpp"
#include "Devices/Sockets/InetSocketAddress.hpp"
#include "sys_call_eval.h"
//#include "sys_call_err_eval.hpp"
#include "ObservableStatePublisher.hpp"
#include "Devices/TestDevice.h"
#include <functional>
#include <utility>

#include "meta.h"
#include "Policies/StateChangeAdvertiserPolicy.hpp"
#include "Policies/DatagramIOPolicy.hpp"
#include "Policies/FifoIOPolicy.hpp"
#include "Devices/Pipes/NamedPipeDevice.hpp"
#include "Devices/Pipes/NamedPipeFactory.h"
#include "Devices/DeviceFactory.hpp"
#include "Devices/DeviceAddressFactory.hpp"
#include "Connector.hpp"
#include "ConnectorClient.h"
#include "Reactor/Reactor.hpp"
#include "Reactor/EventTypes.h"
#include "Reactor/EpollDemultiplexer.h"

#include "template_typelist.hpp"

void testDevicePolicies();
void testSocketDevices();
void testErrorChangedPolicy();
void testSocketDatagramIO();
void testNamedPipes();
void testDeviceFactory();
void testPolicyHolder();
void testGetEventsArray();
void testGenerateEndpointType();
void testConnector();
void testPackHost();
void testDeviceTypeErasure();

namespace infra{
DEFINE_STATE_CHANGE_ADVERTISER_POLICY(AEIOU);
DEFINE_STATE_CHANGE_ADVERTISER_POLICY(Error);
/*
template <typename Device, typename = void>
struct HasObsErr : std::false_type
{};

template <typename Device>
struct HasObsErr<Device, std::enable_if_t<is_observable<std::invoke_result_t<decltype(&ErrorMemberAccess::getMember<Device>),
                                                                             std::reference_wrapper<Device>>>::value>>
  : std::true_type

{};*/

}
/*
class A
{
    friend class BaseAccessor;

    void print() const { std::cout << "accessing private member\n"; }
};

class BaseAccessor
{
    //friend class Client;
protected:
    template<typename T>
    static void printMeWhatINeed(const T & a)
    {
        a.print();
    }
};

class DerivedAccessor : protected BaseAccessor
{
    friend class Client;
    using BaseAccessor::printMeWhatINeed;
};

class Client
{
   public:
     void test()
     {
         A a;
         //BaseAccessor::printMeWhatINeed(a);
         DerivedAccessor::printMeWhatINeed(a);
     }
};


int main() {
    // your code goes here
    Client cl;
    cl.test();
    return 0;
}
*/
struct my_dummy{};
int main()
{
    using namespace infra;
    infra::meta::dispatch::dispatch_main();
    //Connector<my_dummy> rappin_on;
    //using policies_pack = PoliciesHolder<typename Connector<my_dummy>::DeviceConnectionPolicy>;
    //using CustomPipeT = typename policies_pack::AssembledClientT<ReadingNamedPipeDevice<UnixResourceHandler>>;
    /*
    Connector<my_dummy>::test<SocketDevice<UnixResourceHandler,
                 IPV4InetSocketAddress,
                 ipv4_domain,
                 datagram_socket>>();
                 */
    //std::cout << "is socket device: " << IsSocketDeviceT<socketdevt>::value << "\n";
    //std::cout << "is fifo device: " << IsNamedPipeDeviceT<devt>::value << "\n";
    //socket_traits<devt> s;
    //decltype(std::declval<socket_traits<devt>>()) j{};
    //testGetEventsArray();
    //testGenerateEndpointType();
    //testDeviceTypeErasure();

    

    //testNamedPipes();
    //testDeviceFactory();

    //std::cout << HasMemberT_AEIOU<TestDevice>::value << "\n";

    /*
    std::cout << HasObservableErrorMemberT<TestDevice>::value << "\n";
    std::cout << is_observable<std::invoke_result_t<decltype(&ErrorMemberAccess::getMember<TestDevice>),
                                                    std::reference_wrapper<TestDevice>>>::value << "\n";
    std::cout << HasObsErr<TestDevice>::value << "\n";*/

    //testErrorChangedPolicy();
    return 0;
}

using PoliciesSet = infra::meta::ttl::template_typelist<infra::GenericIOPolicy,
                                            infra::AcceptorPolicy,
                                            infra::ErrorChangeAdvertiserPolicy>;
using TransportPoliciesSet = infra::meta::ttl::template_typelist<>;

void testPolicyHolder()
{
    using namespace infra;
    using policies_pack = PoliciesHolder<FifoIOPolicy>;
    using CustomPipeT = typename policies_pack::AssembledClientT<ReadingNamedPipeDevice<UnixResourceHandler>>;
    auto original_rd_pipe = CustomPipeT();

    using SocketT = SocketDevice<UnixResourceHandler, IPV4InetSocketAddress, ipv4_domain, stream_socket>;
    //using CustomSocketT = typename AssembleClient<PoliciesSet, SocketT>::AssembledClientT;
    //auto origina_rude_boy = CustomSocketT();
}

void testPackHost()
{
    using namespace infra;
    using device_t = ReadingNamedPipeDevice<UnixResourceHandler>;
    using policies_t = meta::ttl::template_typelist<SeekableOperations, FifoIOPolicy, ConnectionPolicy>;
    using packed_host_t = PackHostT<device_t, policies_t>;

    using unpacked_device_t = UnpackedClientT<packed_host_t>;
    using unpacked_policies_t = UnpackedPluginsTList<packed_host_t>;
    static_assert(std::is_same_v<device_t, unpacked_device_t>);
    static_assert(std::is_same_v<policies_t, unpacked_policies_t>);
}

void testGenerateEndpointType()
{
    using namespace infra;
    constexpr std::size_t device_tag = static_cast<std::size_t>(EDeviceType::E_IPV6_UDP_SOCKET_DEVICE);
    /*
    using DeviceAddressT = typename DeviceAddressFactory<device_tag>::DeviceAddressT;
    using DeviceConnParamsT = ConnectionParameters<device_tag, DeviceAddressT>;

    using type_gen = transport_traits<UnixResourceHandler,
                                                    DeviceConnParamsT,
                                                    ConnectionPolicy,
                                                    GenericIOPolicy>;
    using devicet_t = typename type_gen::device_t;
    using transport_endpoint_t = typename type_gen::transport_endpoint_t;
    */
    using type_gen2 = transport_traits<device_tag,
                                        UnixResourceHandler,
                                        PoliciesSet,
                                        TransportPoliciesSet>;
    using device_t2 = typename type_gen2::device_t;
    using device_host_t2 = typename type_gen2::device_host_t;
    using transport_endpoint_t2 = typename type_gen2::transport_endpoint_t;
    static_assert(std::is_same_v<SocketDevice<UnixResourceHandler, 
                                              IPV6InetSocketAddress,
                                              ipv6_domain,
                                              datagram_socket>, 
                                 device_t2>);
    
}

void testConnector()
{
    using namespace infra;
    using namespace inet;

    using handleT = int;
    using ReactorT = Reactor<handleT, demux::EpollDemultiplexer<handleT>>;
    constexpr std::size_t device_tag = static_cast<std::size_t>(EDeviceType::E_IPV6_UDP_SOCKET_DEVICE);
    using DeviceAddressT = typename DeviceAddressFactory<device_tag>::DeviceAddressT;
    using DeviceConnParamsT = ConnectionParameters1<device_tag, DeviceAddressT>;
    using type_gen = transport_traits2<UnixResourceHandler, DeviceConnParamsT, ConnectionPolicy, GenericIOPolicy>;
    using device_t = typename type_gen::device_t;

    ReactorT reactor;
    Connector<ReactorT> connector(reactor);

    DeviceConnParamsT params;
    HostAddress<ipv6_domain> host_addr;
    host_addr.setAddressAny();
    NetworkAddress<IPV6HostAddr> network_addr{host_addr, 5668};
    params.addr.setAddress(network_addr);
    std::function<void(std::unique_ptr<infra::AbstractTransportEndpoint>)> cb = [](std::unique_ptr<AbstractTransportEndpoint> ptr){ std::cout << "yey\n"; };

    connector.setup<UnixResourceHandler, DeviceConnParamsT, 
                    ConnectionPolicy, GenericIOPolicy>(params, cb);
}

template<typename Device>
void printBytesAvailable(infra::Host<Device, infra::ResourceStatusPolicy> *p_dev)
{
    std::cout << p_dev->bytesAvailable();
}

void testDeviceTypeErasure()
{
    using namespace infra;
    using device_t = ReadingNamedPipeDevice<UnixResourceHandler>;
    using policies_t = meta::ttl::template_typelist<ResourceStatusPolicy>;
    using packed_host_t = PackHostT<device_t, policies_t>;
    packed_host_t my_device{};
    void *placeholder = reinterpret_cast<void*>(&my_device);
    printBytesAvailable(&my_device);
    printBytesAvailable(reinterpret_cast<packed_host_t*>(placeholder));

    std::string config_path{"/home/bfrancu/Documents/Work/Projects/IPCInfra/Configuration/example.ini"};
    std::string section{"CONNECTION_DETAILS"};
    ConnectorClient<default_client_traits> dl_client{config_path};
    dl_client.init(section);
    int dev_type = dl_client.getDeviceType();
    std::cout << "Dev type read is " << dev_type << "\n";

}

void testDeviceFactory()
{
    using namespace infra;
    auto dev = DeviceFactory<EDeviceType::E_READING_FIFO_DEVICE>::createDevice<UnixResourceHandler, FifoIOPolicy>();
}

void testGetEventsArray()
{
    using namespace infra;
    events_array my_arr = getArray<EHandleEvent::E_HANDLE_EVENT_IN, EHandleEvent::E_HANDLE_EVENT_PRIO_IN,
                                   EHandleEvent::E_HANDLE_EVENT_SHUTDOWN, EHandleEvent::E_HANDLE_EVENT_ERR,
                                   EHandleEvent::E_HANDLE_EVENT_OUT, EHandleEvent::E_HANDLE_EVENT_IN,
                                   EHandleEvent::E_HANDLE_EVENT_LAST>();

    for (EHandleEvent event : my_arr)
    {
        std::cout << "event: " << static_cast<int>(event) << "\n";
    }

}

void testNamedPipes()
{
    using namespace infra;
    NamedPipeFactory factory;
    std::string fifo_path{"/home/bfrancu/Documents/Work/myfifo2"};

    /*
    int flags = 0;
    flags |= O_NONBLOCK;
    flags |= O_RDONLY;


    if (-1 != ::open(fifo_path.c_str(), flags)){
        std::cout << "open pipe succedeed\n";
    }
    else std::cerr << "open pipe failed with " << errno << "\n";

//    struct stat f_stats;
//    if (-1 == ::stat("/home/bfrancu/Documents/Work", &f_stats)){
//        std::cout << "errno " << errno << "\n";
//    }
     */
    //NamedPipeDevice<UnixResourceHandler> pipey;
     //auto original_rd_pipe = factory.getReadingEndpoint<FifoIOPolicy>(fifo_path, false);
    using policies_pack = PoliciesHolder<FifoIOPolicy>;
    using CustomPipeT = typename policies_pack::AssembledClientT<ReadingNamedPipeDevice<UnixResourceHandler>>;
     //auto original_rd_pipe = Host<ReadingNamedPipeDevice<UnixResourceHandler>, FifoIOPolicy>();
    auto original_rd_pipe = CustomPipeT();
     original_rd_pipe.open(fifo_path, false);
     std::string msg;
     original_rd_pipe.read(msg);
     std::cout << "received: " << msg << "\n";

//    ReadingNamedPipeDevice<UnixResourceHandler> reading_pipe;
//    std::cout << std::boolalpha << reading_pipe.open("alabala", true)
//              << "\n";

//    WritingNamedPipeDevice<UnixResourceHandler> writing_pipe(fifo_path);
//    std::cout << std::boolalpha << writing_pipe.open(true)
//              << "\n";
}

void errorPrinter(const infra::io::EFileIOError & err){
    std::cout << "errorPrinter: " << infra::utils::to_underlying(err) << "\n";
}

void testErrorChangedPolicy()
{

    using namespace infra;

    Host<TestDevice, ErrorChangeAdvertiserPolicy> t_dev;


    t_dev.onErrorChangedSubscribe([](auto err){
        std::cout << "errorLambda: " << static_cast<int>(err) << "\n";
    });

    auto sub_id = t_dev.OnErrorChanged += (errorPrinter);

    std::cout << "errorPrinter subscribed with " << sub_id << "\n";

    t_dev.setError(io::EFileIOError::E_ERROR_IO);
    sleep(2);
    t_dev.setError(io::EFileIOError::E_ERROR_DOM);
    sleep(2);

    if (t_dev.OnErrorChanged -= (sub_id)){
        std::cout << "errorPrinter unsubscribed\n";
    }

    t_dev.setError(io::EFileIOError::E_ERROR_INTR);
    sleep(2);
}

void testSocketDatagramIO()
{
    using namespace infra;
    using namespace inet;

    Host<SocketDevice<UnixResourceHandler,
                 IPV4InetSocketAddress,
                 ipv4_domain,
                 datagram_socket>,
            AcceptorPolicy,
            DatagramIOPolicy> ipv4_dgram_sock{};

    IPV4InetSocketAddress binded_addr{IPV4NetworkAddress(IPV4HostAddr::AddressAny(), 5661)};
    if (ipv4_dgram_sock.bind(binded_addr)){
        std::cout << "binded\n";
    }
    else
    {
        std::cerr << "bind errno " << errno << "\n";
        return;
    }

     IPV4InetSocketAddress remote_caller;
     std::string content;

     std::cout << "here\n";
     auto len = ipv4_dgram_sock.recvFrom(512, SocketIOFlags{io::ESocketIOFlag::E_MSG_NO_FLAG}, content, remote_caller);
     std::cout << "after\n";

     if (-1 == len){
         std::cerr << "errno: " << errno << "\n";
     }

     if (len != 0)
     {
         std::cout << "received: " << content << "\n";
     }

     IPV4InetSocketAddress remote_addr{IPV4NetworkAddress(IPV4HostAddr::AddressAny(), 5662)};
     ipv4_dgram_sock.sendTo(remote_addr, content);

    /*
     int sock = ::socket(PF_INET, SOCK_DGRAM, 0);
     sockaddr_in b_addr;
     socklen_t s_len{sizeof(sockaddr_in)};
     memset(&b_addr, 0, sizeof(sockaddr_in));

     b_addr.sin_family = PF_INET;
     b_addr.sin_port = htons(5061);
     b_addr.sin_addr.s_addr = INADDR_ANY;

     if (-1 == ::bind(sock, reinterpret_cast<sockaddr*>(&b_addr), s_len)){
         std::cerr << "binding failed for linux socket with " << errno << "\n";
     }

     sockaddr *p_remote{nullptr};
     socklen_t s_addr_len{sizeof(sockaddr)};
     char buf[512];
     memset(buf, 0 , 512);
     int recv_len = ::recvfrom(sock, buf, 25, 0, p_remote, &s_addr_len);
     std::cout << "size read " << recv_len << ": " << buf << "\n";
*/

}

void testSocketDevices()
{
    using namespace infra;
    using namespace inet;

    using IPV4HostAddr = HostAddress<ipv4_domain>;
    using IPV4NetworkAddress = NetworkAddress<IPV4HostAddr>;
    using IPV4InetSocketAddress = InetSocketAddress<IPV4NetworkAddress>;

    Host<SocketDevice<UnixResourceHandler,
                 IPV4InetSocketAddress,
                 ipv4_domain,
                 stream_socket>,
            AcceptorPolicy> ipv4_stream_sock{};

    SocketDevice<UnixResourceHandler,
                 IPV4InetSocketAddress,
                 ipv4_domain,
                 stream_socket> another_one;

    HostAddress<ipv4_domain> host_addr;

    host_addr.setAddressAny();
    std::cout << "is loopback: " << host_addr.isLoopback()
              << "; is any: " << host_addr.isAny()
              << "\n";


    NetworkAddress<IPV4HostAddr> network_addr{host_addr, 5661} ;
    network_addr.port_number = 5661;
    network_addr.host_address = host_addr;

    IPV4InetSocketAddress inet_addr;
    inet_addr.setAddress(network_addr);


    std::cout << "state before binding: " <<  ipv4_stream_sock.getState() << "\n";
    if (ipv4_stream_sock.bind(inet_addr)){
        std::cout << "state after binding: " << ipv4_stream_sock.getState() << "\n";
    }
    else std::cerr << "binding failed with " << errno << "\n";
    ipv4_stream_sock.listen();
}

void testDevicePolicies()
{
    using namespace infra;
    std::string filename{"/home/bfrancu/Documents/Work/IPCInfra/example.txt"};
    Host<FileDevice<UnixResourceHandler>, GenericIOPolicy,
                                           SeekableOperations,
                                           ResourceStatusPolicy> file{filename};

    if (file.open(io::EAccessMode::E_READ_WRITE))
    {
        std::string content;
        file.seek(4, io::ESeekWhence::E_SEEK_SET);
        file.read(content);
        std::cout << content;
        std::cout << "\n type: " << infra::utils::to_underlying(file.fileType()) << "\n";
        file.close();
    }
}


void testDummyDevice()
{
    struct custom_platform{};

    struct DummyDevice
    {
        using handle_type = char;
        using platform = custom_platform;
        DummyDevice() = default;
        int getHandle() const {return 0;}
        platform i;
    };

    using namespace infra;
    Host<DummyDevice, GenericIOPolicy,
                      SeekableOperations,
                      ResourceStatusPolicy> dummy;
}


void test_sys_call_evaluator()
{
    using namespace infra;
    std::string filename{"/home/bfrancu/Documents/Work/IPCInfra/example.txt"};

    if(sys_call_zero_eval(access, filename.c_str(), F_OK))
    {
        std::cout << "ofc\n";
    }
    else
    {
        std::cerr << "err: " << errno << "\n";
    }
}
