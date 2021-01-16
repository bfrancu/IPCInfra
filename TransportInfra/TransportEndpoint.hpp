#ifndef TRANSPORTENDPOINT_HPP
#define TRANSPORTENDPOINT_HPP
#include "Host.hpp"
#include "Reactor/EventTypes.h"

namespace infra
{

class AbstractTransportEndpoint
{
public:
    virtual ~AbstractTransportEndpoint() = default;
    virtual void *getDevice() = 0;
};

template <typename AssembledDevice, typename Enable = void>
class TransportEndpoint : public AbstractTransportEndpoint
{
    //using device_type = Device;  //Device from <Host<Device, ..>>
public:

    explicit TransportEndpoint()
    {}

    void *getDevice() override
    {
        return reinterpret_cast<void*>(&m_device);
    };

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

    /*
    AssembledDevice & getDevice()
    {
        return m_device;
    }*/

private:
    bool m_InitCompleted;
    AssembledDevice m_device;
};

} //infra
#endif

