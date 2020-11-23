#ifndef TRANSPORTENDPOINT_HPP
#define TRANSPORTENDPOINT_HPP
#include "Reactor/EventTypes.h"

namespace infra
{

/*
 * TODO
 * Replace Device with Host<Device, Policies...>
 */
template <typename Device, typename Enable = void>
class TransportEndpoint 
{
    using device_type = Device;  //Device from <Host<Device, ..>>

public:

    explicit TransportEndpoint()
    {}

public:
    EHandleEventResult handleEvent(EHandleEvent event)
    {}

    template<typename U>
    void setDevice(U && device)
    {
        m_device = std::forward<U>(device);
    }

    // Policy will be a <template<typename, typename> > Policy
    template<typename Policy>
    Policy & getDeviceAs()
    {
        return static_cast<Policy&>(m_device);
    }

    Device & getDevice()
    {
        return m_device;
    }

private:
    bool m_InitCompleted;
    Device m_device;
};

} //infra
#endif

