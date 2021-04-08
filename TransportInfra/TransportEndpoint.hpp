#ifndef TRANSPORTENDPOINT_HPP
#define TRANSPORTENDPOINT_HPP
#include <memory>
#include "Host.hpp"
#include "Reactor/EventTypes.h"
#include "Devices/GenericDeviceAccess.hpp"
#include "Policies/ConnectionStateChangeAdvertiser.hpp"
#include "Observable.hpp"

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
    virtual std::size_t getConnectionState() const = 0;
};

template<typename T>
class DynamicTransportEndpointWrapper : public ITransportEndpoint
{
public:
    DynamicTransportEndpointWrapper(std::unique_ptr<T> p_endpoint) :
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

    std::size_t getConnectionState() const override{
        return static_cast<std::size_t>(m_pEndpoint->getConnectionState());
    }

private:
    std::unique_ptr<T> m_pEndpoint;
};

/*
 * TODO Move The Listener subscribe/handle event logic to a separate policy
 */

template <typename AssembledDevice,
          template<typename...> typename EventHandlingPolicy,
          template<typename...> typename DispatcherPolicy,
          typename Listener,
          typename StateChangeCallbackDispatcher = SerialCallbackDispatcher,
          typename Enable = void>
class TransportEndpoint : public EventHandlingPolicy<TransportEndpoint<AssembledDevice, EventHandlingPolicy, DispatcherPolicy, Listener>, Listener>,
                          public DispatcherPolicy<TransportEndpoint<AssembledDevice, EventHandlingPolicy, DispatcherPolicy, Listener>, AssembledDevice>,
                          public ConnectionStateAdvertiser<TransportEndpoint<AssembledDevice, EventHandlingPolicy, DispatcherPolicy, Listener>, StateChangeCallbackDispatcher>
{
    using EventHandlingBase = EventHandlingPolicy<TransportEndpoint<AssembledDevice, EventHandlingPolicy, DispatcherPolicy, Listener>, Listener>;
    using DispatcherBase = DispatcherPolicy<TransportEndpoint<AssembledDevice, EventHandlingPolicy, DispatcherPolicy, Listener>, AssembledDevice>;

public:
    using Device = AssembledDevice;
    using SubscriberID = typename Listener::SubscriberID;

public:
    explicit TransportEndpoint(Listener & listener) :
        EventHandlingBase(listener)
    {}

public:
    bool onInputEvent()
    {
        return DispatcherBase::ProcessInputEvent();
    }

    bool onDisconnection()
    {
        DispatcherBase::ProcessDisconnection();
        // change state to something specific
        return true;
    }

    bool onErrorEvent()
    {
        // error handling policy?
        m_device.disconnect();
        return true;
    }

    template<typename U>
    void setDevice(U && device)
    {
        m_device = std::forward<U>(device);
        //EventHandlingPolicy<Listener>::setHandle(GenericDeviceAccess::getHandle(m_device));
        EventHandlingBase::setHandle(GenericDeviceAccess::getHandle(m_device));
    }

    // Policy will be a <template<typename, typename> > Policy
    template<typename Policy>
    Policy & getDeviceAs()
    {
        return static_cast<Policy&>(m_device);
    }

    inline AssembledDevice & getDevice() { return m_device; }
    //inline Listener & getListener() { return m_listener; }
    const Observable<std::size_t> & getConnectionState() const { return m_connectionState; }

private:
    Observable<std::size_t> m_connectionState;
    bool m_initCompleted;
    AssembledDevice m_device;
    //Listener & m_listener;
    /*
    SubscriberID m_listener_sub_id{Listener::NULL_SUBSCRIBER_ID};
    */
};

} //infra
#endif

