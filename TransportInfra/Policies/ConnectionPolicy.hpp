#ifndef CONNECTIONPOLICY_HPP
#define CONNECTIONPOLICY_HPP
//#include <sys/un.h>
#include <ios>
#include <sys/socket.h>
#include <fcntl.h>

#include <memory>
#include <iostream>

#include "SocketTypes.h"
#include "sys_call_eval.h"
#include "crtp_base.hpp"
#include "utilities.hpp"
#include "Traits/device_constraints.hpp"
//#include "Devices/Sockets/SocketDeviceAccess.hpp"
//#include "Devices/Pipes/NamedPipeDeviceAccess.hpp"

namespace infra
{

template <typename Host, typename Device>
struct DummyConnectionPolicy : public crtp_base<DummyConnectionPolicy<Host, Device>, Host>
{
    bool connect(...)
    {
        std::cout << "DummyConnectionPolicy::connect()\n";
        return true;
    }

    bool disconnect() {
        std::cout << "DummyConnectionPolicy::disconnect()\n";
        return this->asDerived().close();
    }
};

template<typename Host, typename Device,
         typename = void>
class ConnectionPolicy{};

template<typename Host, typename SocketDevice>
class ConnectionPolicy<Host, traits::SocketDevice<SocketDevice>>
        : public crtp_base<ConnectionPolicy<Host, SocketDevice>, Host>

{
    using handle_t  = typename device_traits<SocketDevice>::handle_type;
    using address_t = typename socket_traits<SocketDevice>::address_type;

public:
    bool connect(const address_t & sock_addr, bool non_blocking = false) {
        std::cout << "ConnectionPolicy::connect() addr: " << sock_addr << "; non blocking: " << std::boolalpha << non_blocking << "\n";
        bool ret{false};

        if (io::ESocketState::E_STATE_DISCONNECTED == this->asDerived().getState()){
            handle_t handle = this->asDerived().getHandle();

            if (handler_traits<SocketDevice>::defaultValue() == handle) return ret;

            this->asDerived().setHandle(handle);
            this->asDerived().setState(io::ESocketState::E_STATE_AVAILABLE);
        }

        if (io::ESocketState::E_STATE_AVAILABLE != this->asDerived().getState()) return ret;

        auto handle = this->asDerived().getHandle();
        if (non_blocking)
        {
            int flags = ::fcntl(handle, F_GETFL);
            bool res = (-1 != ::fcntl(handle, F_SETFL, flags | O_NONBLOCK));
            if (!res)
            {
                std::cout << "ConnectionPolicy::connect() fcntl SOCK_NONBLOCK failure\n";
                return ret;
            }
        }

        std::unique_ptr<sockaddr> p_addr{std::make_unique<sockaddr>()};
        sock_addr.getAddress(*p_addr);

        ret = sys_call_zero_eval(::connect,
                                 handle,
                                 p_addr.get(),
                                 static_cast<socklen_t>(sock_addr.getAddressLength()));
        //auto res = ::connect(handle, p_addr.get(), static_cast<socklen_t>(sock_addr.getAddressLength()));
        //ret = (0 == res);

        std::cout << "ConnectionPolicy::connect() ret: " << ret << " errno: " << errno << "\n";
        if (non_blocking && (EINPROGRESS == errno) && !ret)
        {
            std::cout << "ConnectionPolicy::connect() operation in progress\n";
            ret = true;
        }

        if (ret) this->asDerived().setState(io::ESocketState::E_STATE_CONNECTED);

        return ret;
    }

    bool disconnect(){
        std::cout << "ConnectionPolicy::disconnect()\n";
        if (io::ESocketState::E_STATE_CONNECTED != this->asDerived().getState()) return false;
        if (this->asDerived().close()){
            this->asDerived().setState(io::ESocketState::E_STATE_DISCONNECTED);
            return true;
        }
        return false;
    }

    bool shutdown(io::EShutdownHow how){
        bool ret = sys_call_noerr_eval(::shutdown, this->asDerived().getHandle(), how);
        if (ret) this->asDerived().setState(io::ESocketState::E_STATE_SHUTDOWN);

        return ret;
    }

    bool isConnected() const { return io::ESocketState::E_STATE_CONNECTED == this->asDerived().getState(); }

protected:
    ~ConnectionPolicy() = default;
};

template<typename Host, typename Device>
class ConnectionPolicy<Host, Device, std::enable_if_t<IsNamedPipeDeviceT<Device>::value && std::negation_v<IsSocketDeviceT<Device>>>>
                 : public crtp_base<ConnectionPolicy<Host, Device>, Host>
{
    using address_t = typename fifo_traits<Device>::address_type; 
    using io_profile_t   = typename fifo_traits<Device>::io_profile;

public:
    bool connect(const address_t & addr, bool non_blocking = false) {
        return this->asDerived().open(addr, non_blocking);
    }

    bool disconnect() {
        std::cout << "ConnectionPolicy::disconnect()\n";
        return this->asDerived().close();
    }

    bool shutdown(io::EShutdownHow how) {
       static constexpr bool read_only = std::is_same_v<io_profile_t, read_only_profile>;
       if (io::EShutdownHow::E_SHUTDOWN_WRITE == how && read_only){
           return false;
       }

       disconnect();
       return true;
    }

    bool isConnected() const { return this->asDerived().isOpen(); }

protected:
    ~ConnectionPolicy() = default;
};

} // infra

#endif // CONNECTIONPOLICY_HPP
