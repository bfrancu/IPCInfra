#ifndef SOCKETDEVICE_HPP
#define SOCKETDEVICE_HPP
#include <sys/un.h>
#include <netinet/ip.h>

#include "SocketTypes.h"
#include "FileIODefinitions.h"
#include "utilities.hpp"
#include "Observable.hpp"
#include "Traits/socket_traits.hpp"
#include "SocketDeviceAccess.hpp"

//#include "Infra/sys_call_err_eval.hpp"

#include <iostream>

namespace infra
{

template<typename ResourceHandler, typename SocketAddress>
class SocketDeviceBase
{
    friend class SocketDeviceAccess;
    //friend class PlatformApiProxy;

public:
    using handle_type = typename handler_traits<ResourceHandler>::handle_type;

public:
    SocketDeviceBase() :
        m_resource_handler{},
        m_working_socket_address{},
        m_state{io::ESocketState::E_STATE_AVAILABLE}
    {}

    io::ESocketState getState() const { return m_state; }    

protected:
    friend class ErrorMemberAccess;
    const Observable<io::EFileIOError> & getError() const {return m_last_io_error;}

    /*
    template<typename Device, typename cb, typename... Args>
    friend std::enable_if_t<std::negation_v<HasMemberT_SetLatestError<Device>>, bool>
    SysCallZeroEval(Device &, cb, Args&&...);

    template<typename Device, typename cb, typename... Args>
    friend std::enable_if_t<HasMemberT_SetLatestError<Device>::value, bool>
    SysCallZeroEval(Device &, cb, Args&&...);
    */

    //template<typename IODevice, typename Sys_callable, typename... Args>
    //friend bool SysCallZeroEval(IODevice & device, Sys_callable f, Args&&... args);

    void setLatestError(int error_val) {
        std::cout << "SocketDeviceBase::setLatestError\n";
        static auto err_last_val{utils::to_underlying(io::EFileIOError::E_ERROR_LAST)};
        static auto err_first_val{utils::to_underlying(io::EFileIOError::E_ERROR_FIRST)};
        if (err_last_val > error_val && err_first_val < error_val){ m_last_io_error = static_cast<io::EFileIOError>(error_val); }
    }

protected:
    ~SocketDeviceBase() = default;
    handle_type getHandle() const { return m_resource_handler.getHandle(); }
    io::EFileIOError getLatestIOError() const { return m_last_io_error; }
    void setHandle(handle_type handle) { m_resource_handler.acquire(handle); }
    void setState(io::ESocketState state) { m_state = state; }
    template<typename SA>
    void setWorkingAddress(SA && sock_addr) { m_working_socket_address = std::forward<SA>(sock_addr);}
    void close(){ m_resource_handler.close(); }

protected:
    ResourceHandler m_resource_handler;
    SocketAddress m_working_socket_address; // will be set by the connection/acceptor policies
    io::ESocketState m_state;
    Observable<io::EFileIOError> m_last_io_error {0};

};


template <typename ResourceHandler,
          typename SocketAddress,
          typename Domain,
          typename ConnectionProfile,          
          typename = void>
class SocketDevice{};


template <typename ResourceHandler, typename SocketAddress>
class SocketDevice<ResourceHandler,
                   SocketAddress,
                   unix_domain,
                   stream_socket,
                   std::enable_if_t<UnixSocketRequires<ResourceHandler, SocketAddress>::value>>
        : public SocketDeviceBase<ResourceHandler, SocketAddress>
{    

public:    
    using handle_type         = typename handler_traits<ResourceHandler>::handle_type;
    using platform            = typename handler_traits<ResourceHandler>::platform;
    using socket_domain       = unix_domain;
    using socket_type         = stream_socket;
    using socket_address_type = SocketAddress;

public:
    static handle_type createSocketHandle() { std::cout << "unix stream\n"; return ::socket(PF_UNIX, SOCK_STREAM, 0); }

public:
    SocketDevice() { this->setHandle(createSocketHandle()); }

    template <typename SD>
    std::enable_if_t<CompatibleSocketDevices<SocketDevice, SD>::value>
    assign(SocketDevice && socket_device){
       *this = std::forward<SD>(socket_device);
    }
};


template <typename ResourceHandler, typename SocketAddress>
class SocketDevice<ResourceHandler,
                   SocketAddress,
                   unix_domain,
                   datagram_socket,
                   std::enable_if_t<UnixSocketRequires<ResourceHandler, SocketAddress>::value>>
        : public SocketDeviceBase<ResourceHandler, SocketAddress>
{

public:
    using handle_type         = typename handler_traits<ResourceHandler>::handle_type;
    using platform            = typename handler_traits<ResourceHandler>::platform;
    using socket_domain       = unix_domain;
    using socket_type         = datagram_socket;
    using socket_address_type = SocketAddress;

public:
    static handle_type createSocketHandle() { std::cout << "unix dgram\n"; return ::socket(PF_UNIX, SOCK_DGRAM, 0); }

public:
    SocketDevice() {this->setHandle(createSocketHandle()); }

    template <typename SD>
    std::enable_if_t<CompatibleSocketDevices<SocketDevice, std::decay_t<SD>>::value>
    assign(SD && socket_device){
       *this = std::forward<SD>(socket_device);
    }
};


template <typename ResourceHandler, typename SocketAddress>
class SocketDevice<ResourceHandler,
                   SocketAddress,
                   ipv4_domain,
                   datagram_socket,
                   std::enable_if_t<IPV4SocketRequires<ResourceHandler, SocketAddress>::value>>
        : public SocketDeviceBase<ResourceHandler, SocketAddress>
{

public:
    using handle_type         = typename handler_traits<ResourceHandler>::handle_type;
    using platform            = typename handler_traits<ResourceHandler>::platform;
    using socket_domain       = ipv4_domain;
    using socket_type         = datagram_socket;
    using socket_address_type = SocketAddress;

public:
    static handle_type createSocketHandle() { std::cout << "ipv4 udp\n"; return ::socket(PF_INET, SOCK_DGRAM, 0); }

public:
    SocketDevice() { this->setHandle(createSocketHandle()); }

    template <typename SD>
    std::enable_if_t<CompatibleSocketDevices<SocketDevice, std::decay_t<SD>>::value>
    assign(SD && socket_device){
       *this = std::forward<SD>(socket_device);
    }
};


template <typename ResourceHandler, typename SocketAddress>
class SocketDevice<ResourceHandler,
                   SocketAddress,
                   ipv4_domain,
                   stream_socket,
                   std::enable_if_t<IPV4SocketRequires<ResourceHandler, SocketAddress>::value>>
        : public SocketDeviceBase<ResourceHandler, SocketAddress>
{

public:
    using handle_type         = typename handler_traits<ResourceHandler>::handle_type;
    using platform            = typename handler_traits<ResourceHandler>::platform;
    using socket_domain       = ipv4_domain;
    using socket_type         = stream_socket;
    using socket_address_type = SocketAddress;

public:
    static handle_type createSocketHandle() { std::cout << "ipv4 tcp\n"; return ::socket(PF_INET, SOCK_STREAM, 0); }

public:
    SocketDevice() { this->setHandle(createSocketHandle()); }

    template <typename SD>
    std::enable_if_t<CompatibleSocketDevices<SocketDevice, std::decay_t<SD>>::value>
    assign(SD && socket_device){
       *this = std::forward<SD>(socket_device);
    }
};


template <typename ResourceHandler, typename SocketAddress>
class SocketDevice<ResourceHandler,
                   SocketAddress,
                   ipv6_domain,
                   datagram_socket,
                   std::enable_if_t<IPV6SocketRequires<ResourceHandler, SocketAddress>::value>>
        : public SocketDeviceBase<ResourceHandler, SocketAddress>
{

public:
    using handle_type         = typename handler_traits<ResourceHandler>::handle_type;
    using platform            = typename handler_traits<ResourceHandler>::platform;
    using socket_domain       = ipv6_domain;
    using socket_type         = datagram_socket;
    using socket_address_type = SocketAddress;

public:
    static handle_type createSocketHandle() { std::cout << "ipv6 udp\n"; return ::socket(PF_INET6, SOCK_DGRAM, 0); }

public:
     SocketDevice() { this->setHandle(createSocketHandle()); }

    template <typename SD>
    std::enable_if_t<CompatibleSocketDevices<SocketDevice, std::decay_t<SD>>::value>
    assign(SD && socket_device){
       *this = std::forward<SD>(socket_device);
    }
};


template <typename ResourceHandler, typename SocketAddress>
class SocketDevice<ResourceHandler,
                   SocketAddress,
                   ipv6_domain,
                   stream_socket,
                   std::enable_if_t<IPV6SocketRequires<ResourceHandler, SocketAddress>::value>>
        : public SocketDeviceBase<ResourceHandler, SocketAddress>
{

public:
    using handle_type         = typename handler_traits<ResourceHandler>::handle_type;
    using platform            = typename handler_traits<ResourceHandler>::platform;
    using socket_domain       = ipv6_domain;
    using socket_type         = stream_socket;
    using socket_address_type = SocketAddress;

public:
    static handle_type createSocketHandle() { std::cout << "ipv6 tcp\n"; return ::socket(PF_INET6, SOCK_STREAM, 0); }

public:
    SocketDevice() { this->setHandle(createSocketHandle()); }

    template <typename SD>
    std::enable_if_t<CompatibleSocketDevices<SocketDevice, std::decay_t<SD>>::value>
    assign(SD && socket_device){
       *this = std::forward<SD>(socket_device);
    }
};

} //infra
#endif // SOCKETDEVICE_HPP
