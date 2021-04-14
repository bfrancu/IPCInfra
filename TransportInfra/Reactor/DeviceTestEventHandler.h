#ifndef DEVICETESTEVENTHANDLER_H
#define DEVICETESTEVENTHANDLER_H
#include <iostream>

#include "EventHandlerSubscriber.h"
#include "Policies/EventHandlingPolicy.hpp"

namespace infra
{
template<typename Device, typename Listener>
class DeviceTestEventHandler : public infra::BaseEventHandlingPolicy<DeviceTestEventHandler<Device, Listener>, Listener>
{
    using Base = infra::BaseEventHandlingPolicy<DeviceTestEventHandler<Device, Listener>, Listener>;
public:
    DeviceTestEventHandler(Device & device, Listener & listener) :
        Base(listener),
        m_device(device)
    {}

    void init()
    {
        Base::setHandle(infra::GenericDeviceAccess::getHandle(m_device));
    }

    bool onInputEvent()
    {
        std::cout << "DeviceTestEventHandler::onInputEvent()\n";
        ssize_t size_read = m_device.read(m_local_buffer);
        if (-1 == size_read){
            this->asDerived().onErrorEvent();
            return false;
        }

        std::cout << "DeviceTestEventHandler::onInputEvent() read from device: " << m_local_buffer << "\n";
        m_local_buffer.clear();
        return true;
    }

    bool onDisconnection()
    {
        std::cout << "DeviceTestEventHandler::onDisconnection()\n";
        return true;
    }

    bool onErrorEvent()
    {
        std::cout << "DeviceTestEventHandler::onErrorEvent()\n";
        return true;
    }

private:
    Device & m_device;
    std::string m_local_buffer{};
};

}//infra

#endif // DEVICETESTEVENTHANDLER_H
