#ifndef CLIENT_CALLBACK_POLICY_HPP
#define CLIENT_CALLBACK_POLICY_HPP
#include <atomic>
#include <type_traits>
#include <iostream>

#include "typelist.hpp"

#include "Devices/DeviceDefinitions.h"
#include "Traits/utilities_traits.hpp"

namespace infra
{

template<typename Derived, typename Device, typename Storage = meta::tl::empty_type, typename = void>
class ClientCallbackPolicy
{
    /*
     * TODO uncomment the static_assert; delete the rest of the code in this class
    static_assert(HasConnectableDevice<Derived>::value, 
                  "The Derived class should expose a Device type which provides the \"connect\" and \"disconnect\" methods in its interface");
    */
public:
    bool connect(...) { return true; }
    bool disconnect() { return true; }
    bool ProcessInputEvent() { return true; }
    bool ProcessOutputEvent() { return true; }
    bool ProcessHangupEvent() { return true; }
    bool ProcessErrorEvent()  { return true; }
    bool ProcessDisconnectionEvent() { return true; }
};

template<typename Derived, typename Device>
class ClientCallbackPolicy<Derived, Device, meta::tl::empty_type, std::enable_if_t<IsConnectableDeviceT<Device>::value>>
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

    bool disconnect()
    {
        if (this->asDerived().getDevice().disconnect()){
            this->asDerived().setState(EConnectionState::E_DISCONNECTED);
            //m_connected = false;
            return true;
        }
        return false;
    }

    void onConnected()
    {
        std::cout << "ClientCallbackPolicy::onConnected()\n";
        if (m_connectionInProgress){
            m_connectionInProgress.store(false);
            //m_connected = true;
            this->asDerived().setState(EConnectionState::E_CONNECTED);
        }
    }

    template<typename... Args>
    ssize_t send(Args&&... args)
    {
        return sendImpl<Device>(this->asDerived().getDevice(), std::forward<Args>(args)...);
    }

protected:
    ssize_t sendImpl(...){ return -1; }

    template<typename Dev, typename... Args, 
             typename = decltype(std::declval<std::decay_t<Dev>>().send(std::declval<Args&&>()...))>
    std::enable_if_t<IsSendableDeviceT<Dev>::value, ssize_t> 
    sendImpl(Dev & device, Args&&... args)
    {
        return device.send(std::forward<Args>(args)...);
    }

    template<typename Dev, typename... Args,
             typename = void, typename = decltype(std::declval<std::decay_t<Dev>>().write(std::declval<Args&&>()...))>
    std::enable_if_t<(IsWritableDeviceT<Dev>::value && !IsSendableDeviceT<Dev>::value), ssize_t> 
    sendImpl(Dev & device, Args&&... args)
    {
        return device.write(std::forward<Args>(args)...);
    }

    bool ProcessInputEvent()
    {
        std::cout << "ClientCallbackPolicy::ProcessInputEvent()\n";
        return true;
    }

    bool ProcessOutputEvent()
    {
        std::cout << "ClientCallbackPolicy::ProcessOutputEvent()\n";
        if (/*!m_connected &&*/this->asDerived().getConnectionState() == static_cast<std::size_t>(EConnectionState::E_CONNECTING)){
            onConnected();
            std::cout << "ClientCallbackPolicy::ProcessOutputEvent(): connection established\n";
        }
        /*
        else
        {
            std::cout << "ClientCallbackPolicy::ProcessOutputEvent() not connected yet. connected: " << m_connected
                      << "; connection state: " << this->asDerived().getConnectionState() << "\n";
        }
        */
        return true;
    }

    bool ProcessHangupEvent()
    {
        std::cout << "ClientCallbackPolicy::ProcessHangupEvent()\n";
        return true;
    }

    bool ProcessErrorEvent() 
    {
        if (static_cast<size_t>(EConnectionState::E_CONNECTED) == this->asDerived().getConnectionState()){
            this->asDerived().setState(EConnectionState::E_DISCONNECTING);
            return disconnect();
        }
        return true;
    }

    bool ProcessDisconnectionEvent()
    {
        std::cout << "ClientCallbackPolicy::ProcessDisconnectionEvent()\n";
        size_t endpoint_state = this->asDerived().getConnectionState();
        if (static_cast<size_t>(EConnectionState::E_DISCONNECTED) != endpoint_state)
        {
            this->asDerived().setState(EConnectionState::E_DISCONNECTED);
        }
        //m_connected = false;
        return true;
    }

protected:
    ~ClientCallbackPolicy() = default;

    Derived & asDerived() { return static_cast<Derived &>(*this); }
    const Derived & asDerived() const { return static_cast<const Derived &>(*this); }

private:
    std::atomic<bool> m_connectionInProgress{false};
    //bool m_connected{false};
};

}//infra

#endif
