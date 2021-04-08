#ifndef DEVICETESTEVENTHANDLER_H
#define DEVICETESTEVENTHANDLER_H
#include <iostream>

#include "EventHandlerSubscriber.h"
#include "Policies/EventHandlingPolicy.hpp"

template<typename Device, typename Listener>
class DeviceTestEventHandler : public infra::BaseEventHandlingPolicy<DeviceTestEventHandler<Device, Listener>, Listener>
{
    using Base = infra::BaseEventHandlingPolicy<DeviceTestEventHandler<Device, Listener>, Listener>;
public:
    DeviceTestEventHandler(Device & device, Listener & listener) :
        Base(listener),
        m_device(device)
    {}

    bool onInputEvent()
    {
        std::cout << "DeviceTestEventHandler::onInputEvent()\n";
    }

    bool onDisconnection()
    {
        std::cout << "DeviceTestEventHandler::onDisconnection()\n";
    }

    bool onErrorEvent()
    {
        std::cout << "DeviceTestEventHandler::onErrorEvent()\n";
    }



private:
    Device & m_device;
};


#endif // DEVICETESTEVENTHANDLER_H
