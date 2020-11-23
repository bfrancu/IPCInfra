#ifndef CONNECTIONPOLICY_HPP
#define CONNECTIONPOLICY_HPP
//#include <sys/un.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <memory>
#include <iostream>

#include "SocketTypes.h"
#include "sys_call_eval.h"
#include "crtp_base.hpp"
#include "utilities.hpp"
#include "Traits/socket_traits.hpp"
#include "Traits/fifo_traits.hpp"
#include "Devices/Sockets/SocketDeviceAccess.hpp"
#include "Devices/Pipes/NamedPipeDeviceAccess.hpp"

namespace infra
{

template<typename Host, typename Device,
         typename = void>
class ConnectionPolicy{};

template<typename Host, typename SocketDevice>
class ConnectionPolicy<Host, SocketDevice, std::enable_if_t<IsUnixSocketDeviceT<SocketDevice>::value>>
        : public crtp_base<ConnectionPolicy<Host, SocketDevice,
                                            std::enable_if_t<IsUnixSocketDeviceT<SocketDevice>::value>>,
                           Host>

{
    using handle_type  = typename device_traits<SocketDevice>::handle_type;
    using address_type = typename socket_traits<SocketDevice>::address_type;

public:
    bool connect(const address_type & sock_addr, bool non_blocking = false) {
        std::cout << "ConnectionPolicy::connect()\n";
        bool ret{false};

        if (io::ESocketState::E_STATE_DISCONNECTED == this->asDerived().getState()){
            handle_type handle = SocketDeviceAccess::createSocketHandle<SocketDevice>();

            if (handler_traits<SocketDevice>::defaultValue() == handle) return ret;

            SocketDeviceAccess::setHandle(this->asDerived(), handle);
            SocketDeviceAccess::setState(this->asDerived(), io::ESocketState::E_STATE_AVAILABLE);
        }

        if (io::ESocketState::E_STATE_AVAILABLE != this->asDerived().getState()) return ret;

        if (non_blocking && !sys_call_noerr_eval(::fcntl,
                                               SocketDeviceAccess::getHandle(this->asDerived()),
                                               SOCK_NONBLOCK)) return ret; 

        std::unique_ptr<sockaddr> p_addr{std::make_unique<sockaddr>()};
        sock_addr.getAddress(*p_addr);

        ret = sys_call_zero_eval(::connect,
                               SocketDeviceAccess::getHandle(this->asDerived()),
                               p_addr.get(),
                               static_cast<socklen_t>(sock_addr.getAddressLength())
                               );
        std::cout << "ConnectionPolicy::connect() ret: " << ret << "\n";
        if (ret) SocketDeviceAccess::setState(this->asDerived(), io::ESocketState::E_STATE_CONNECTED);

        return ret;
    }

    void disconnect(){
        if (io::ESocketState::E_STATE_CONNECTED != this->asDerived().getState()) return;
        SocketDeviceAccess::close(this->asDerived());
        SocketDeviceAccess::setState(this->asDerived(), io::ESocketState::E_STATE_DISCONNECTED);
    }

    bool shutdown(io::EShutdownHow how){
        bool ret = sys_call_noerr_eval(::shutdown, this->asDerived().getHandle(), how);
        if (ret) SocketDeviceAccess::setState(this->asDerived(), io::ESocketState::E_STATE_SHUTDOWN);

        return ret;
    }

    bool isConnected() const { return io::ESocketState::E_STATE_CONNECTED == this->asDerived().getState(); }
};

template<typename Host, typename Device>
class ConnectionPolicy<Host, Device, std::void_t<std::enable_if_t<IsNamedPipeDeviceT<Device>::value &&
                                                                  std::negation_v<IsSocketDeviceT<Device>>>>>
                 : public crtp_base<ConnectionPolicy<Host, Device>, Host>
{
    using handle_type  = typename device_traits<Device>::handle_type; 
    using address_type = typename fifo_traits<Device>::address_type; 
    using io_profile   = typename fifo_traits<Device>::io_profile;

public:
    bool connect(const address_type & addr, bool non_blocking = false) {
        return this->asDerived().open(addr, non_blocking);
    }

    void disconnect() {
        NamedPipeDeviceAccess::close(this->asDerived());
    }

    bool shutdown(io::EShutdownHow how) {
       static constexpr bool read_only = std::is_same_v<io_profile, read_only_profile>;
       if (io::EShutdownHow::E_SHUTDOWN_WRITE == how && read_only){
           return false;
       }

       disconnect();
       return true;
    }

    bool isConnected() const { return this->asDerived().isOpen(); }
};

} // infra

#endif // CONNECTIONPOLICY_HPP
