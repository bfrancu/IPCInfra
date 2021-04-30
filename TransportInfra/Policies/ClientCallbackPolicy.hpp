#ifndef CLIENT_CALLBACK_POLICY_HPP
#define CLIENT_CALLBACK_POLICY_HPP
#include <atomic>
#include <type_traits>

#include "Devices/DeviceDefinitions.h"
#include "Traits/utilities_traits.hpp"

namespace infra
{

template<typename Derived, typename Device, typename = void>
class ClientCallbackPolicy
{
    static_assert(HasConnectableDevice<Derived>::value, 
                  "The Derived class should expose a Device type which provides the \"connect\" and \"disconnect\" methods in its interface");
};

template<typename Derived, typename Device>
class ClientCallbackPolicy<Derived, Device, std::enable_if_t<IsConnectableDeviceT<Device>::value>>
{
   // using Device = typename Derived::Device;
    using address_t = typename Device::address_type;

public:
    bool connect(const address_t & addr, bool non_blocking)
    {
       Device & device = this->asDerived().getDevice();
       bool result = device.connect(addr, non_blocking);
       if (non_blocking && result){
           m_connectionInProgress.store(true);
           this->asDerived().setState(EConnectionState::E_CONNECTING);
       }
       return result;
    }

    void disconnect()
    {
        this->asDerived().getDevice().disconnect();
        this->asDerived().setState(EConnectionState::E_DISCONNECTING);
    }

protected:
    bool ProcessInputEvent()
    {
        return true;
    }

    bool ProcessOutputEvent()
    {
        if (m_connectionInProgress){
            m_connectionInProgress.store(false);
            this->asDerived().setState(EConnectionState::E_CONNECTED);
        }
        return true;
    }

    bool ProcessErrorEvent() 
    {
        disconnect();
        return true;
    }

    bool ProcessDisconnectionEvent()
    {
        size_t endpointState = this->asDerived().getConnectionState();
        if (static_cast<size_t>(EConnectionState::E_DISCONNECTED) != endpointState)
        {
            this->asDerived().setState(EConnectionState::E_DISCONNECTED);
        }
        return true;
    }

protected:
    ~ClientCallbackPolicy() = default;

    Derived & asDerived() { return static_cast<Derived &>(*this); }
    const Derived & asDerived() const { return static_cast<const Derived &>(*this); }

private:
    std::atomic<bool> m_connectionInProgress{false};
};

}//infra

#endif
