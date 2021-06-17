#ifndef ACCEPTORPOLICY_H
#define ACCEPTORPOLICY_H
#include <sys/socket.h>

#include <memory>

#include "SocketTypes.h"
#include "FileInfo.h"
#include "LinuxUtils/LinuxIOUtilities.h"
#include "crtp_base.hpp"
#include "sys_call_eval.h"
#include "Traits/device_constraints.hpp"
#include "Devices/Sockets/SocketDeviceAccess.hpp"

namespace infra
{

template<typename Host, typename SocketDevice,
         typename = void>
class AcceptorPolicy {};


template<typename Host, typename SocketDevice,
         typename = void>
class SocketAcceptorBasePolicy;

template<typename Host, typename SocketDevice>
class SocketAcceptorBasePolicy<Host, traits::UnixPlatformSocketDevice<SocketDevice>> :
    public crtp_base<SocketAcceptorBasePolicy<Host, SocketDevice>, Host>
{
protected:
    using handle_t = typename device_traits<SocketDevice>::handle_type;
    using address_t = typename socket_traits<SocketDevice>::address_type;

protected:
    bool bind(const address_t& sock_addr, bool reusable){
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

    bool listen(int backlog){
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

    inline bool isBinded() const { return io::ESocketState::E_STATE_BINDED == this->asDerived().getState(); }
    inline bool isListening() const { return io::ESocketState::E_STATE_LISTENING == this->asDerived().getState(); }
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

    std::optional<SocketDevice> accept(bool non_blocking) {
        std::optional<SocketDevice> peer_sock;
        if (io::ESocketState::E_STATE_LISTENING != this->asDerived().getState()) return peer_sock;

        sockaddr remote_addr;
        socklen_t remote_addr_len{sizeof(sockaddr)};

        int flags{0};
        if (non_blocking) flags |= SOCK_NONBLOCK;

        handle_t peer_sock_handle = ::accept4(SocketDeviceAccess::getHandle(this->asDerived()), &remote_addr, &remote_addr_len, flags);

        if (handler_traits<SocketDevice>::defaultValue() != peer_sock_handle){
             address_t peer_sock_addr;
             peer_sock_addr.setAddress(remote_addr);
             peer_sock.emplace();

             SocketDeviceAccess::setWorkingAddress(peer_sock.value(), std::move(peer_sock_addr));
             SocketDeviceAccess::setHandle(peer_sock.value(), peer_sock_handle);
             SocketDeviceAccess::setState(peer_sock.value(), io::ESocketState::E_STATE_CONNECTED);
        }
        else{
            SocketDeviceAccess::setState(peer_sock.value(), io::ESocketState::E_STATE_ERROR);
        }
        return peer_sock;
    }

protected:
    ~SocketAcceptorBasePolicy() = default;
};

template <typename Host, typename SocketDevice>
class AcceptorPolicy<Host, SocketDevice, std::enable_if_t<IsUnixPlatformSocketDeviceT<SocketDevice>::value &&
                                                          !IsUnixSocketDeviceT<SocketDevice>::value>>
        : public SocketAcceptorBasePolicy<Host, SocketDevice>
{
protected:
    using Base = SocketAcceptorBasePolicy<Host, SocketDevice>;
    //using handle_t = typename Base::handle_type;
    using address_t = typename Base::address_t;

public:
    bool bind(const address_t& sock_addr, bool reusable = true){ return Base::bind(sock_addr, reusable); }
    bool listen(int backlog = 50){ return Base::listen(backlog); }
    decltype(auto) accept(bool non_blocking = false)  { return Base::accept(non_blocking); }

    inline bool isBinded() const { return Base::isBinded(); }
    inline bool isListening() const{ return Base::isListening(); }
    bool isAddressReusable() const{ return Base::isAddressReusable(); }
    bool setReusableAddressOpt(bool reusable){ return Base::setReusableAddressOpt(reusable); }

protected:
    ~AcceptorPolicy() = default;
};

template<typename Host, typename SocketDevice>
class AcceptorPolicy<Host, SocketDevice, std::enable_if_t<IsUnixSocketDeviceT<SocketDevice>::value>>
       : public SocketAcceptorBasePolicy<Host, SocketDevice>
{
protected:
    using Base = SocketAcceptorBasePolicy<Host, SocketDevice>;
    //using handle_t = typename Base::handle_type;
    using address_t = typename Base::address_t;

public:
    using test_type = int;

    bool bind(const address_t & sock_addr, bool reusable = true){
        sockaddr_un tmp_sockaddr;
        memset(&tmp_sockaddr, 0, sizeof(sockaddr_un));
        sock_addr.getAddress(tmp_sockaddr);
        m_socket_pathname.reserve(sizeof(tmp_sockaddr.sun_path));
        m_socket_pathname.assign(tmp_sockaddr.sun_path);

        return Base::bind(sock_addr, reusable); 
    }

    bool listen(int backlog = 50){ return Base::listen(backlog); }
    std::optional<SocketDevice> accept(bool non_blocking = false)  { return Base::accept(non_blocking); }

    inline bool isBinded() const { return Base::isBinded(); }
    inline bool isListening() const{ return Base::isListening(); }
    bool isAddressReusable() const{ return Base::isAddressReusable(); }
    bool setReusableAddressOpt(bool reusable){ return Base::setReusableAddressOpt(reusable); }

protected:
    ~AcceptorPolicy()
    {
        if (!m_socket_pathname.empty())
        {
            if (utils::unx::LinuxIOUtilities::exists(m_socket_pathname))
            {
                io::FileInfo fileInfo{m_socket_pathname};
                if (io::EFileType::E_LOCAL_SOCKET == fileInfo.type())
                {
                    utils::unx::LinuxIOUtilities::remove(m_socket_pathname);
                }
            }
            m_socket_pathname.clear();
        }
    }

private:
    std::string m_socket_pathname{};
};


template<typename Host, typename Device>
class AcceptorPolicy<Host, Device, std::enable_if_t<IsNamedPipeDeviceT<Device>::value &&
                                                    std::negation_v<IsSocketDeviceT<Device>>>>
         : public crtp_base<AcceptorPolicy<Host, Device>, Host>
{
    //using handle_type  = typename device_traits<Device>::handle_type;
    using address_t = typename fifo_traits<Device>::address_type;

public:
    bool bind(const address_t& addr, bool = true) { return this->asDerived().setAddress(addr); }
    bool listen(int) { return true; }
    std::optional<Device> accept(bool non_blocking = false) {
        this->asDerived().open(non_blocking);
        return {};
    }

protected:
    ~AcceptorPolicy() = default;
};

} //infra

#endif // ACCEPTORPOLICY_H
