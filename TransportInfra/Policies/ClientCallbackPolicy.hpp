#ifndef CLIENT_CALLBACK_POLICY_HPP
#define CLIENT_CALLBACK_POLICY_HPP
#include <atomic>
#include <type_traits>
#include <iostream>

#include "Devices/DeviceDefinitions.h"
#include "Traits/utilities_traits.hpp"

namespace infra
{

template<typename Derived, typename Device, typename = void>
class ClientCallbackPolicy
{
    /*
     * TODO uncomment the static_assert; delete the rest of the code in this class
    static_assert(HasConnectableDevice<Derived>::value, 
                  "The Derived class should expose a Device type which provides the \"connect\" and \"disconnect\" methods in its interface");
    */
public:
    bool connect(...) { return true; }
    void disconnect();
    bool ProcessInputEvent() { return true; }
    bool ProcessOutputEvent() { return true; }
    bool ProcessHangupEvent() { return true; }
    bool ProcessErrorEvent()  { return true; }
    bool ProcessDisconnectionEvent() { return true; }
};

template<typename Derived, typename Device>
class ClientCallbackPolicy<Derived, Device, std::enable_if_t<IsConnectableDeviceT<Device>::value>>
{
   // using Device = typename Derived::Device;
    using address_t = typename Device::address_type;

public:
    bool connect(const address_t & addr, bool non_blocking)
    {
        std::cout << "ClientCallbackPolicy::connect()\n";
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
        m_connected = false;
    }

    void onConnected()
    {
        std::cout << "ClientCallbackPolicy::onConnected()\n";
        if (m_connectionInProgress){
            m_connectionInProgress.store(false);
            m_connected = true;
            this->asDerived().setState(EConnectionState::E_CONNECTED);
        }
    }

protected:
    bool ProcessInputEvent()
    {
        std::cout << "ClientCallbackPolicy::ProcessInputEvent()\n";
        return true;
    }

    bool ProcessOutputEvent()
    {
        if (!m_connected){
            onConnected();
            std::cout << "ClientCallbackPolicy::ProcessOutputEvent(): connection established\n";
        }
        return true;
    }

    bool ProcessHangupEvent()
    {
        std::cout << "ClientCallbackPolicy::ProcessHangupEvent()\n";
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
    bool m_connected{false};
};

}//infra

#endif
