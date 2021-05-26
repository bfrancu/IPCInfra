#ifndef TRANSPORTENDPOINT_HPP
#define TRANSPORTENDPOINT_HPP
#include <memory>
#include "typelist.hpp"
#include "utilities.hpp"
#include "Host.hpp"
#include "Reactor/EventTypes.h"
#include "Devices/GenericDeviceAccess.hpp"
#include "Devices/DeviceDefinitions.h"
#include "Policies/ConnectionStateChangeAdvertiser.hpp"
#include "Observable.hpp"
#include "BaseMemberPair.hpp"

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

class IClientTransportEndpoint : public ITransportEndpoint
{
public:
    virtual void onConnected() = 0;
    virtual bool disconnect() = 0;
    virtual bool connect(std::string_view addr, bool non_blocking) = 0;
    //virtual ssize_t send(std::string_view data) = 0;
};

template<typename T, typename AbstractInterface = ITransportEndpoint>
class DynamicTransportEndpointWrapper : public AbstractInterface
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
        //std::cout << "DynamicTransportEndpointWrapper::listenerUnsubscribe()\n";
        return m_pEndpoint->listenerUnsubscribe();
    }
    
    bool subscribedToListener() const override{
        return m_pEndpoint->subscribedToListener();
    }

    std::size_t getConnectionState() const override{
        return static_cast<std::size_t>(m_pEndpoint->getConnectionState());
    }

protected:
    std::unique_ptr<T> m_pEndpoint;
};

template<typename T>
class ClientDynamicTransportEndpointWrapper : public DynamicTransportEndpointWrapper<T, IClientTransportEndpoint>
{
public:
    ClientDynamicTransportEndpointWrapper(std::unique_ptr<T> p_endpoint) :
        DynamicTransportEndpointWrapper<T, IClientTransportEndpoint>(std::move(p_endpoint))
    {}

    void onConnected() override {
        this->m_pEndpoint->onConnected();
    }

    bool disconnect() override {
        return this->m_pEndpoint->disconnect();
    }

    bool connect(std::string_view addr, bool non_blocking) override{
        typename T::DeviceAddress dev_addr;
        //TODO Create overload in DeviceAddressFactry that returns a string from the DeviceAddress
        if (dev_addr.fromString(addr)){
            return this->m_pEndpoint->connect(dev_addr, non_blocking);
        }
        return false;
    }

    /*
    ssize_t send(std::string_view data) override{
        return this->m_pEndpoint->send(data);
    }
    */
};
/*
 * TODO Move The Listener subscribe/handle event logic to a separate policy
 */

template <typename AssembledDevice,
          template<typename...> typename EventHandlingPolicy,
          template<typename...> typename DispatcherPolicy,
          template<typename...> typename ClientServerRolePolicy,
          typename Listener,
          typename Storage = meta::tl::empty_type,
          typename StateChangeCallbackDispatcher = SerialCallbackDispatcher,
          typename Enable = void>
class TransportEndpoint : public EventHandlingPolicy<TransportEndpoint<AssembledDevice, EventHandlingPolicy, DispatcherPolicy, ClientServerRolePolicy, Listener>, Listener>,
                          public DispatcherPolicy<TransportEndpoint<AssembledDevice, EventHandlingPolicy, DispatcherPolicy, ClientServerRolePolicy, Listener>, AssembledDevice>,
                          public ClientServerRolePolicy<TransportEndpoint<AssembledDevice, EventHandlingPolicy, DispatcherPolicy, ClientServerRolePolicy, Listener>, AssembledDevice>,
                          public ConnectionStateAdvertiser<TransportEndpoint<AssembledDevice, EventHandlingPolicy, DispatcherPolicy, ClientServerRolePolicy, Listener>, StateChangeCallbackDispatcher>
{
    using EventHandlingBase = EventHandlingPolicy<TransportEndpoint<AssembledDevice, EventHandlingPolicy, DispatcherPolicy, ClientServerRolePolicy, Listener>, Listener>;
    using DispatcherBase = DispatcherPolicy<TransportEndpoint<AssembledDevice, EventHandlingPolicy, DispatcherPolicy, ClientServerRolePolicy,  Listener>, AssembledDevice>;
    using ClientServerLogicBase = ClientServerRolePolicy<TransportEndpoint<AssembledDevice, EventHandlingPolicy, DispatcherPolicy, ClientServerRolePolicy, Listener>, AssembledDevice>;

    friend ClientServerLogicBase;
    friend EventHandlingBase;

public:
    using Device = AssembledDevice;
    using DeviceAddress = typename Device::address_type;
    using SubscriberID = typename Listener::SubscriberID;

public:
    explicit TransportEndpoint(Listener & listener) :
        EventHandlingBase(listener)
    {
        m_optional_storage_and_state.member() = utils::to_underlying(EConnectionState::E_AVAILABLE);
    }

public:
    bool onInputEvent()
    {
        return ClientServerLogicBase::ProcessInputEvent() && DispatcherBase::ProcessInputEvent();
    }

    bool onWriteAvailable() 
    {
        return ClientServerLogicBase::ProcessOutputEvent();
    };

    bool onDisconnection()
    {
        std::cout << "TransportEndpoint::onDisconnection()\n";
        //DispatcherBase::ProcessDisconnection();
        return ClientServerLogicBase::ProcessDisconnectionEvent();
    }

    bool onHangupEvent()
    {
        return ClientServerLogicBase::ProcessHangupEvent();
    }

    bool onErrorEvent()
    {
        // error handling policy?
        return ClientServerLogicBase::ProcessErrorEvent();
    }

    void setDevice(AssembledDevice device)
    {
        m_device = std::move(device);
        //EventHandlingPolicy<Listener>::setHandle(GenericDeviceAccess::getHandle(m_device));
        EventHandlingBase::setHandle(GenericDeviceAccess::getHandle(m_device));
    }

public:
    inline Device & getDevice() { return m_device; }
    inline const Device & getDevice() const { return m_device; }
    inline const Observable<std::size_t> & getConnectionState() const { return m_optional_storage_and_state.member(); }
    inline Observable<std::size_t> & getConnectionState() { return m_optional_storage_and_state.member(); }

protected:
    void setState(EConnectionState state)
    {
        std::cout << "TransportEndpoint::setState() new state: " << utils::to_underlying(state) << "\n";
        std::size_t old_state{m_optional_storage_and_state.member()};
        m_optional_storage_and_state.member() = utils::to_underlying(state);
        if(EConnectionState::E_DISCONNECTED == state && old_state != utils::to_underlying(EConnectionState::E_DISCONNECTED))
        {
            DispatcherBase::ProcessDisconnection();
        }
    }

    inline auto getHandle() const { return GenericDeviceAccess::getHandle(m_device); }

private:
    BaseMemberPair<Storage, Observable<std::size_t>> m_optional_storage_and_state;
    bool m_initCompleted;
    AssembledDevice m_device;
    /*
    SubscriberID m_listener_sub_id{Listener::NULL_SUBSCRIBER_ID};
    */
};

} //infra
#endif

