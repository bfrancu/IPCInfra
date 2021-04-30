#ifndef DEVICETESTEVENTHANDLER_H
#define DEVICETESTEVENTHANDLER_H
#include <iostream>
#include <mutex>
#include <atomic>

#include "EventHandlerSubscriber.h"
#include "Traits/device_constraints.hpp"
#include "Policies/EventHandlingPolicy.hpp"
#include "SocketTypes.h"

namespace infra
{
template<typename Device, typename Listener>
class DeviceTestEventHandler : public infra::BaseEventHandlingPolicy<DeviceTestEventHandler<Device, Listener>, Listener>
{
    using Base = infra::BaseEventHandlingPolicy<DeviceTestEventHandler<Device, Listener>, Listener>;
    using address_t = typename Device::address_type;
    //using traits_t = traits::select_traits<Device>;

public:
    DeviceTestEventHandler(Device & device, Listener & listener) :
        Base(listener),
        m_device(device)
    {}

    void init()
    {
        Base::setHandle(infra::GenericDeviceAccess::getHandle(m_device));
    }

    bool connect(const address_t & addr, bool non_blocking)
    {
        std::cout << "DeviceTestEventHandler::connect()\n";
        bool result = m_device.connect(addr, non_blocking);
        if (non_blocking && result){
            m_connectionInProgress.store(true);
        }
        return result;
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
        return true;
    }

    bool onErrorEvent()
    {
        std::cout << "DeviceTestEventHandler::onErrorEvent() an error or an hangup ocurred\n";
        return true;
    }

    bool onWriteAvailable()
    {
        if (m_connectionInProgress){
            m_connectionInProgress.store(false);
            std::cout << "DeviceTestEventHandler::onWriteAvailable() Connection established\n";
        }

        std::cout << "DeviceTestEventHandler::onWriteAvailable()\n";
        std::string result{"ping\n"};
        //m_device.write(result, io::ESocketIOFlag::E_MSG_NOSIGNAL);
        m_device.write("ping\n");
        return true;
    }

    inline Device & getDevice() { return m_device; }

private:
    Device & m_device;
    std::string m_local_buffer{};
    std::once_flag m_connectionEstablishedFlag;
    std::atomic<bool> m_connectionInProgress{false};
};

}//infra

#endif // DEVICETESTEVENTHANDLER_H
