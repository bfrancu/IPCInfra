#ifndef TRANSPORTENDPOINT_HPP
#define TRANSPORTENDPOINT_HPP
#include <memory>
#include "Host.hpp"
#include "Reactor/EventTypes.h"
#include "Devices/GenericDeviceAccess.hpp"

namespace infra
{

class ITransportEndpoint
{
public:
    virtual ~ITransportEndpoint() = default;
    virtual void *getDevice() = 0;
    virtual void *getInternalEndpoint() = 0;
    virtual void *releaseInternalEndpoint() = 0;
    virtual bool listenerSubscribe(const events_array & events) = 0;
    virtual bool listenerUnsubscribe() = 0;
    virtual bool subscribedToListener() const = 0;
};

template<typename T>
class DynamicTransportEndpointAdaptor : public ITransportEndpoint
{
public:
    DynamicTransportEndpointAdaptor(std::unique_ptr<T> p_endpoint) :
        m_pEndpoint(std::move(p_endpoint))
    {}

    void *getDevice() override{
        return m_pEndpoint ? reinterpret_cast<void*>(&(m_pEndpoint->getDevice())) 
                           : nullptr;
    }

    void *getInternalEndpoint() override{
        return reinterpret_cast<void*>(m_pEndpoint.get());
    }

    void *releaseInternalEndpoint() override{
        return reinterpret_cast<void*>(m_pEndpoint.release());
    }

    bool listenerSubscribe(const events_array & events) override {
        return m_pEndpoint->listenerSubscribe(events);
    }

    bool listenerUnsubscribe() override {
        return m_pEndpoint->listenerUnsubscribe();
    }
    
    bool subscribedToListener() const override{
        return m_pEndpoint->subscribedToListener();
    }

private:
    std::unique_ptr<T> m_pEndpoint;
};

/*
 * TODO Move The Listener subscribe/handle event logic to a separate policy
 */

template <typename AssembledDevice,
          template<typename...> typename EventHandlingPolicy,
          typename Listener,
          typename Enable = void>
class TransportEndpoint : public EventHandlingPolicy<Listener>
{
    using SubscriberID = typename Listener::SubscriberID;
 
public:

    explicit TransportEndpoint(Listener & listener) :
        EventHandlingPolicy<Listener>(listener)
    {}
    
    /*
    void *getDevice() override
    {
        return reinterpret_cast<void*>(&m_device);
    };
    */

public:
    /*
    EHandleEventResult handleEvent(SubscriberID id, EHandleEvent event)
    {
        (void) event;
        if (m_listener_sub_id != id)
        {
            return EHandleEventResult::E_RESULT_INVALID_REFERENCE;
        }
        return EHandleEventResult::E_RESULT_DEFAULT;
    }

    inline Listener & getListener() const { return m_listener; }
    inline void setListener(Listener & listener) { m_listener = listener; }
    inline bool subscribedToListener() const { return Listener::NULL_SUBSCRIBER_ID != m_listener_sub_id; }

    bool listenerSubscribe(const events_array & events)
    {
        if (auto id = m_listener.subscribe(events, GenericDeviceAccess::getHandle(m_device), *this);
            Listener::NULL_SUBSCRIBER_ID != id)
        {
            m_listener_sub_id = id;
            return true;
        }
        return false;
    }

    bool listenerUnsubscribe()
    {
        if (m_listener.unsubscribe(m_listener_sub_id))
        {
            m_listener_sub_id = Listener::NULL_SUBSCRIBER_ID;
            return true;
        }
        return false;
    }
    */

    template<typename U>
    void setDevice(U && device)
    {
        m_device = std::forward<U>(device);
        EventHandlingPolicy<Listener>::setHandle(GenericDeviceAccess::getHandle(m_device));
    }

    // Policy will be a <template<typename, typename> > Policy
    template<typename Policy>
    Policy & getDeviceAs()
    {
        return static_cast<Policy&>(m_device);
    }

    AssembledDevice & getDevice()
    {
        return m_device;
    }

private:
    bool m_initCompleted;
    AssembledDevice m_device;
    /*
    Listener & m_listener;
    SubscriberID m_listener_sub_id{Listener::NULL_SUBSCRIBER_ID};
    */
};

} //infra
#endif

