#ifndef ACCEPTORPOLICY_H
#define ACCEPTORPOLICY_H
#include <sys/socket.h>

#include <memory>

#include "SocketTypes.h"

#include "crtp_base.hpp"
#include "sys_call_eval.h"
#include "Traits/socket_traits.hpp"
#include "Traits/fifo_traits.hpp"
#include "Devices/Sockets/SocketDeviceAccess.hpp"
#include "Devices/Pipes/NamedPipeDeviceAccess.hpp"

namespace infra
{

template<typename Host, typename SocketDevice,
         typename = void>
class AcceptorPolicy {};


template <typename Host, typename SocketDevice>
class AcceptorPolicy<Host, SocketDevice, std::enable_if_t<IsUnixSocketDeviceT<SocketDevice>::value>>
        : public crtp_base<AcceptorPolicy<Host, SocketDevice>, Host>
{
    using handle_type  = typename device_traits<SocketDevice>::handle_type;
    using address_type = typename socket_traits<SocketDevice>::address_type;

public:
    bool bind(const address_type & sock_addr, bool reusable = true){
        if (io::ESocketState::E_STATE_AVAILABLE != this->asDerived().getState()) return false;

        setReusableAddressOpt(reusable);
        std::unique_ptr<sockaddr> p_addr{std::make_unique<sockaddr>()};
        sock_addr.getAddress(*p_addr);

        if (sys_call_zero_eval(::bind,
                               SocketDeviceAccess::getHandle(this->asDerived()),
                               p_addr.get(),
                               static_cast<socklen_t>(sock_addr.getAddressLength())))
        {
            SocketDeviceAccess::setState(this->asDerived(), io::ESocketState::E_STATE_BINDED);
            return true;
        }

        return false;
    }

    bool listen(int backlog = 50){
        if (io::ESocketState::E_STATE_BINDED != this->asDerived().getState()) return false;

        if (sys_call_zero_eval(::listen,
                               SocketDeviceAccess::getHandle(this->asDerived()),
                               backlog))
        {
            SocketDeviceAccess::setState(this->asDerived(), io::ESocketState::E_STATE_LISTENING);
            return true;
        }

        return false;
    }

    SocketDevice accept(bool non_blocking = false){
        return acceptImpl(non_blocking);
    }

    /*
    SocketDevice acceptNonBlock(){
        return accept(true);
    }
    */

    bool isBinded() const { return io::ESocketState::E_STATE_BINDED == this->asDerived().getState(); }
    bool isListening() const { return io::ESocketState::E_STATE_LISTENING == this->asDerived().getState(); }
    bool isAddressReusable() const {
        int opt_val{0};
        socklen_t val_size = static_cast<socklen_t>(sizeof(opt_val));
        if (sys_call_noerr_eval(::getsockopt,
                                SocketDeviceAccess::getHandle(this->asDerived()),
                                SOL_SOCKET,
                                SO_REUSEADDR,
                                &opt_val,
                                &val_size)){
            return 1 == val_size;
        }
        return false;
    }

    bool setReusableAddressOpt(bool reusable){
        int opt_val{reusable? 1: 0};
        return sys_call_noerr_eval(::setsockopt,
                                   SocketDeviceAccess::getHandle(this->asDerived()),
                                   SOL_SOCKET,
                                   SO_REUSEADDR,
                                   &opt_val,
                                   sizeof(opt_val));
    }

protected:
    ~AcceptorPolicy() = default;

protected:
    SocketDevice acceptImpl(bool non_blocking) {
        SocketDevice peer_sock;
        if (io::ESocketState::E_STATE_LISTENING != this->asDerived().getState()) return peer_sock;

        sockaddr remote_addr;
        socklen_t remote_addr_len{sizeof(sockaddr)};

        int flags{0};
        if (non_blocking) flags |= SOCK_NONBLOCK;

        handle_type peer_sock_handle = ::accept4(SocketDeviceAccess::getHandle(this->asDerived()), &remote_addr, &remote_addr_len, flags);

        if (handler_traits<SocketDevice>::defaultValue() != peer_sock_handle){
             address_type peer_sock_addr;
             peer_sock_addr.setAddress(remote_addr);

             SocketDeviceAccess::setWorkingAddress(peer_sock, std::move(peer_sock_addr));
             SocketDeviceAccess::setHandle(peer_sock, peer_sock_handle);
             SocketDeviceAccess::setState(peer_sock, io::ESocketState::E_STATE_CONNECTED);
        }
        else{
            SocketDeviceAccess::setState(peer_sock, io::ESocketState::E_STATE_ERROR);
        }
        return peer_sock;
    }
};

template<typename Host, typename Device>
class AcceptorPolicy<Host, Device, std::enable_if_t<IsNamedPipeDeviceT<Device>::value &&
                                                    std::negation_v<IsSocketDeviceT<Device>>>>
         : public crtp_base<AcceptorPolicy<Host, Device>, Host>
{
    using handle_type  = typename device_traits<Device>::handle_type;
    using address_type = typename fifo_traits<Device>::address_type;

public:
    bool bind(const address_type & addr) { return this->asDerived().setAddress(addr); }
    bool listen(int) { return true; }
    bool accept(bool non_blocking = false) { return this->asDerived().open(non_blocking); }

protected:
    ~AcceptorPolicy() = default;
};

} //infra

#endif // ACCEPTORPOLICY_H
