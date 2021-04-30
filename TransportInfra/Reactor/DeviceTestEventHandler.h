#ifndef DEVICETESTEVENTHANDLER_H
#define DEVICETESTEVENTHANDLER_H
#include <iostream>
#include <mutex>
#include <atomic>

#include "EventHandlerSubscriber.h"
#include "Traits/device_constraints.hpp"
#include "Policies/EventHandlingPolicy.hpp"
#include "Policies/ClientCallbackPolicy.hpp"
#include "SocketTypes.h"

namespace infra
{
template<typename Device, typename Listener>
class DeviceTestEventHandler : public infra::BaseEventHandlingPolicy<DeviceTestEventHandler<Device, Listener>, Listener>,
                               public infra::ClientCallbackPolicy<DeviceTestEventHandler<Device, Listener>, Device>
{
    using EventHandlingBase = infra::BaseEventHandlingPolicy<DeviceTestEventHandler<Device, Listener>, Listener>;
    using ClientServerLogicBase = infra::ClientCallbackPolicy<DeviceTestEventHandler<Device, Listener>, Device>;

    using address_t = typename Device::address_type;
    //using traits_t = traits::select_traits<Device>;
    friend ClientServerLogicBase;

public:
    DeviceTestEventHandler(Device & device, Listener & listener) :
        EventHandlingBase(listener),
        m_device(device)
    {}

    void init()
    {
        EventHandlingBase::setHandle(infra::GenericDeviceAccess::getHandle(m_device));
    }

    bool onInputEvent()
    {
        std::cout << "DeviceTestEventHandler::onInputEvent()\n";
        ssize_t size_read = m_device.read(m_local_buffer);
        if (-1 == size_read){
            onErrorEvent();
            return false;
        }

        std::cout << "DeviceTestEventHandler::onInputEvent() read from device: " << m_local_buffer << "\n";
        m_local_buffer.clear();
        return true;
    }

    bool onDisconnection()
    {
        std::cout << "DeviceTestEventHandler::onDisconnection()\n";
        return ClientServerLogicBase::ProcessDisconnectionEvent();
    }

    bool onErrorEvent()
    {
        std::cout << "DeviceTestEventHandler::onErrorEvent() an error or an hangup ocurred\n";
        return true;
    }

    bool onWriteAvailable()
    {
        return ClientServerLogicBase::ProcessOutputEvent();
        /*
        if (m_connectionInProgress){
            m_connectionInProgress.store(false);
            std::cout << "DeviceTestEventHandler::onWriteAvailable() Connection established\n";
        }
        */

        std::cout << "DeviceTestEventHandler::onWriteAvailable()\n";
        std::string result{"ping\n"};
        //m_device.write(result, io::ESocketIOFlag::E_MSG_NOSIGNAL);
        m_device.write("ping\n");
        return true;
    }

    inline Device & getDevice() { return m_device; }
    inline std::size_t & getConnectionState() { return m_connectionState; }

protected:
    void setState(EConnectionState state)
    {
        m_connectionState = static_cast<std::size_t>(state);
    }

private:
    std::size_t m_connectionState;
    Device & m_device;
    std::string m_local_buffer{};
    //std::once_flag m_connectionEstablishedFlag;
    //std::atomic<bool> m_connectionInProgress{false};
};

}//infra

#endif // DEVICETESTEVENTHANDLER_H
