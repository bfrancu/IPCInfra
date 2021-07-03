#ifndef ENDPOINTCONNECTIONINITIALISERBASE_HPP
#define ENDPOINTCONNECTIONINITIALISERBASE_HPP
#include <memory>
#include <unordered_map>
#include <functional>

#include "pointer_traits.hpp"

#include "Devices/DeviceFactory.hpp"
#include "Devices/AccessibleHandleBase.h"
#include "Reactor/EventTypes.h"

namespace infra
{

//TODO Add Locking policy -> if needed
template<typename Derived, typename Demultiplexer, typename IEndpointInterface, typename Enable = void>
class EndpointConnectionInitialiserBase
{

protected:
    using CompletionCallback = std::function<void(std::unique_ptr<IEndpointInterface>&&)>;
    struct ClientEndpointSubscriber
    {
        ClientEndpointSubscriber() = default;
        ClientEndpointSubscriber(std::unique_ptr<IEndpointInterface> p_endpoint, CompletionCallback cb) :
            p_wrapped_endpoint{std::move(p_endpoint)},
            completion_callback{cb}
        {}

        std::unique_ptr<IEndpointInterface> p_wrapped_endpoint{nullptr};
        CompletionCallback completion_callback{nullptr};
    };

    using SubscribersMap = std::unordered_map<typename Demultiplexer::SubscriberID, ClientEndpointSubscriber>;
    using SubscriberIter = typename SubscribersMap::iterator;

public:
    using SubscriberID = typename Demultiplexer::SubscriberID;

    EndpointConnectionInitialiserBase(Demultiplexer & eventDemultiplexer) :
        m_demultiplexer(eventDemultiplexer)
    {}

    EHandleEventResult handleEvent(SubscriberID id, EHandleEvent event)
    {
        auto client_it = m_active_subscriptions.find(id);
        if(m_active_subscriptions.end() == client_it)
        {
            return EHandleEventResult::E_RESULT_INVALID_REFERENCE;
        }

        auto result = this->asDerived().handleEventImpl(client_it, event);
        switch(result)
        {
            case EHandleEventResult::E_RESULT_SUCCESS : this->asDerived().handleConnectionSuccess(client_it); break;
            case EHandleEventResult::E_RESULT_FAILURE : this->asDerived().handleConnectionFailure(client_it); break;
            default: break;
        }
        return result;
    }

    template<typename TransportTraits, typename... DeviceConstructorArgs>
    bool setup(const typename TransportTraits::device_address_t & addr, const CompletionCallback & cb, DeviceConstructorArgs&&... args)
    {
        return this->asDerived().template setupImpl<TransportTraits>(addr, cb, std::forward<DeviceConstructorArgs>(args)...);
    }

protected:
    template<typename TransportTraits, typename Listener, typename... DeviceConstructorArgs>
    static decltype(auto) factorEndpoint(Listener & listener, DeviceConstructorArgs&&... args)
    {
        using resource_handler_t = typename TransportTraits::resource_handler_t;
        using device_policies_t = typename TransportTraits::device_policies_t;
        using transport_endpoint_t = typename TransportTraits::transport_endpoint_t;

        auto p_endpoint = std::make_unique<transport_endpoint_t>(listener);
        p_endpoint->setDevice(DeviceFactory<TransportTraits::device_tag>::template
                              createDevice<resource_handler_t, device_policies_t>(std::forward<DeviceConstructorArgs>(args)...));
        return p_endpoint;
    }

    template<typename ConcreteWrapper, typename Endpoint>
    bool subscribe(const CompletionCallback & cb, std::unique_ptr<Endpoint> p_endpoint, const events_array & subscribed_events)
    {
        std::cout << "EndpointConnectionInitialiserBase::subscribe()\n";
        using handle_t = typename Demultiplexer::Handle;
        handle_t handle{meta::traits::default_value<handle_t>::value};
        auto accessor = [&handle] (handle_t h) { handle = h; };
        AccessKey<decltype(accessor)> handle_access_key;
        handle_access_key.template retrieve(p_endpoint->getDevice(), accessor);

        SubscriberID id = m_demultiplexer.subscribe(subscribed_events, handle, *this);

        if (NULL_SUBSCRIBER_ID != id)
        {
            auto p_endpoint_wrapper = meta::traits::static_cast_unique_ptr<ConcreteWrapper, IEndpointInterface>(
                std::make_unique<ConcreteWrapper>(std::move(p_endpoint)));

            m_active_subscriptions[id] = ClientEndpointSubscriber{std::move(p_endpoint_wrapper), cb};
            return true;
        }

        return false;
    }

    void handleConnectionSuccess(SubscriberIter client_it)
    {
        m_demultiplexer.unsubscribe(client_it->first);
        if (this->asDerived().postConnectionAction(client_it->second.p_wrapped_endpoint))
        {
            this->asDerived().completeClientService(client_it);
        }
    }

    void handleConnectionFailure(SubscriberIter client_it)
    {
        m_demultiplexer.unsubscribe(client_it->first);
        client_it->second.p_wrapped_endpoint.reset(nullptr);
        this->asDerived().completeClientService(client_it);
    }

    void completeClientService(SubscriberIter client_it)
    {
        auto & client_sub = client_it->second;
        client_sub.completion_callback(std::move(client_sub.p_wrapped_endpoint));
        m_active_subscriptions.erase(client_it);
    }

    Demultiplexer & getListener() { return m_demultiplexer; }
    SubscribersMap & getSubscribers() { return m_active_subscriptions; }
    Derived & asDerived() { return static_cast<Derived&>(*this); }
    const SubscribersMap & getSubscribers() const { return m_active_subscriptions; }
    const Derived & asDerived() const { return static_cast<const Derived &>(*this); }

protected:
    static constexpr SubscriberID NULL_SUBSCRIBER_ID{Demultiplexer::NULL_SUBSCRIBER_ID};

protected:
    SubscribersMap m_active_subscriptions;
    Demultiplexer & m_demultiplexer;
};

}//infra
#endif // ENDPOINTCONNECTIONINITIALISERBASE_HPP
