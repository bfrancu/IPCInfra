#ifndef CONNECTOR_HPP
#define CONNECTOR_HPP
#include <functional>
#include <type_traits>
#include <unordered_map>
#include <memory>

#include "enum_flag.h"
#include "Reactor/SubscriberInfo.hpp"
#include "Reactor/EventTypes.h"
#include "Devices/DeviceFactory.hpp"
#include "Policies/ConnectionPolicy.hpp"
#include "Traits/transport_traits.hpp"
#include "PoliciesHolder.hpp"
#include "TransportEndpoint.hpp"
#include "pointer_traits.hpp"
#include "default_traits.hpp"

#include "EndpointConnectionInitialiserBase.hpp"

namespace infra
{

template<typename Demultiplexer, typename Enable = void>
class Connector2 : public EndpointConnectionInitialiserBase<Connector2<Demultiplexer>, Demultiplexer, IClientTransportEndpoint>
{
    struct fifo_device_tag{};
    struct non_fifo_device_tag{};

    using Base = EndpointConnectionInitialiserBase<Connector2<Demultiplexer>, Demultiplexer, IClientTransportEndpoint>;
    using CompletionCallback = typename Base::CompletionCallback;
    using SubscriberID = typename Base::SubscriberID;
    using SubscriberIter = typename Base::SubscriberIter;

    friend class EndpointConnectionInitialiserBase<Connector2<Demultiplexer>, Demultiplexer, IClientTransportEndpoint>;

public:
    Connector2(Demultiplexer & eventDemultiplexer) :
        Base(eventDemultiplexer)
    {}

    Connector2(const Connector2 &) = delete;
    Connector2 & operator=(const Connector2 &) = delete;

protected:
    template<typename Endpoint, typename DeviceAddress>
    bool connect(const CompletionCallback & cb, const DeviceAddress & addr, std::unique_ptr<Endpoint> p_endpoint, non_fifo_device_tag)
    {
        bool non_blocking{true};
        if (p_endpoint->connect(addr, non_blocking))
        {
            std::cout << "Connector::connect(non_fifo_device_tag) device connection done with success\n";
            using ConcreteWrapper = ClientDynamicTransportEndpointWrapper<Endpoint>;
            events_array subscribed_events = getArray<EHandleEvent::E_HANDLE_EVENT_OUT>();
            return Base::template subscribe<ConcreteWrapper>(cb, std::move(p_endpoint), subscribed_events);
        }
        return false;
    }

    template<typename Endpoint, typename DeviceAddress>
    bool connect(const CompletionCallback & cb, const DeviceAddress & addr, std::unique_ptr<Endpoint> p_endpoint, fifo_device_tag)
    {
        using ConcreteWrapper = ClientDynamicTransportEndpointWrapper<Endpoint>;
        bool non_blocking{true};
        bool ret{false};
        std::unique_ptr<IClientTransportEndpoint> p_endpoint_wrapper{nullptr};

        if (p_endpoint->connect(addr, non_blocking))
        {
            std::cout << "Connector::connect(fifo_device_tag) device connection done with success\n";
            p_endpoint_wrapper = meta::traits::static_cast_unique_ptr<ConcreteWrapper, IClientTransportEndpoint>( std::make_unique<ConcreteWrapper>(std::move(p_endpoint)));

            postConnectionAction(p_endpoint_wrapper);
            ret = true;
        }

        cb(std::move(p_endpoint_wrapper));
        return ret;
    }

protected:
    template<typename TransportTraits, typename... DeviceConstructorArgs>
    bool setupImpl(const typename TransportTraits::device_address_t & addr, const CompletionCallback & cb, DeviceConstructorArgs&&... args)
    {
        using device_t = typename TransportTraits::device_t;
        auto p_endpoint = Base::template factorEndpoint<TransportTraits>(Base::getListener(), std::forward<DeviceConstructorArgs>(args)...) ;

        if (p_endpoint)
        {
            return connect(cb, addr, std::move(p_endpoint), traits::select_if_t<IsNamedPipeDeviceT<device_t>,
                                                                                fifo_device_tag,
                                                                                non_fifo_device_tag>{});
        }

        return false;
    }

    EHandleEventResult handleEventImpl(SubscriberIter, EHandleEvent event)
    {
        if (EHandleEvent::E_HANDLE_EVENT_OUT == event)
        {
            return EHandleEventResult::E_RESULT_SUCCESS;
        }

        if (enum_flag(event) & (enum_flag(EHandleEvent::E_HANDLE_EVENT_ERR) |
                                enum_flag(EHandleEvent::E_HANDLE_EVENT_HUP)))
        {
            return EHandleEventResult::E_RESULT_FAILURE;
        }

        return EHandleEventResult::E_RESULT_DEFAULT;
    }


    void handleConnectionSuccess(SubscriberIter client_it)
    {
        std::cout << "Connector::handleConnectionSuccess\n";
        Base::handleConnectionSuccess(client_it);
    }

    void handleConnectionFailure(SubscriberIter client_it)
    {
        std::cout << "Connector::handleConnectionFailure\n";
        Base::handleConnectionFailure(client_it);
    }

    bool postConnectionAction(std::unique_ptr<IClientTransportEndpoint> & p_endpoint_wrapper)
    {
        p_endpoint_wrapper->onConnected();
        events_array events = getArray<EHandleEvent::E_HANDLE_EVENT_IN>();
        p_endpoint_wrapper->listenerSubscribe(events);
        return true;
    }
};
    //using CompletionCallback = std::function<void()>;

    /* TODO
     *
     * Move connection logic to alternative Sync/Async Connection Policy
     * Make the Async Conn policy thread safe
     */
    template<typename Demultiplexer, typename Enable = void>
    class Connector
    {
        struct fifo_device_tag{};
        struct non_fifo_device_tag{};

     public:

        struct ConnectorClientSubscriber;
        //using CompletionCallback = void(*)(std::unique_ptr<IClientTransportEndpoint>&&);
        using CompletionCallback = std::function<void(std::unique_ptr<IClientTransportEndpoint>&&)>;
        using SubscriberID = typename Demultiplexer::SubscriberID;
        using SubscribersMap = std::unordered_map<SubscriberID, ConnectorClientSubscriber>;
        using SubscriberIter = typename SubscribersMap::iterator;
        using Listener = Demultiplexer;

        struct ConnectorClientSubscriber
        {
            ConnectorClientSubscriber() = default;
            ConnectorClientSubscriber(std::unique_ptr<IClientTransportEndpoint> p_endpoint, CompletionCallback cb) :
                p_wrapped_endpoint{std::move(p_endpoint)},
                completion_callback{cb}
            {}

            std::unique_ptr<IClientTransportEndpoint> p_wrapped_endpoint{nullptr};
            CompletionCallback completion_callback{nullptr};
        };

     public:
        Connector(Demultiplexer & eventDemultiplexer) :
            m_demultiplexer(eventDemultiplexer)
        {}

        Connector(const Connector &) = delete;
        Connector & operator=(const Connector &) = delete;

     public:
        template<typename TransportTraits, typename... DeviceConstructorArgs>
        bool setup(const typename TransportTraits::device_address_t & addr,
                   const CompletionCallback & cb, DeviceConstructorArgs&&... args)
        {
            std::cout << "Connector::setup() connecting device; address: " << addr << "\n";
            using resource_handler_t = typename TransportTraits::resource_handler_t;
            using device_policies_t = typename TransportTraits::device_policies_t;
            using device_t = typename TransportTraits::device_t;
            using transport_endpoint_t = typename TransportTraits::transport_endpoint_t;

            auto p_endpoint = std::make_unique<transport_endpoint_t>(m_demultiplexer);
            p_endpoint->setDevice(DeviceFactory<TransportTraits::device_tag>::template 
                                  createDevice<resource_handler_t, device_policies_t>(std::forward<DeviceConstructorArgs>(args)...));

            return connect(cb, addr, std::move(p_endpoint), traits::select_if_t<IsNamedPipeDeviceT<device_t>,
                                                                                fifo_device_tag, 
                                                                                non_fifo_device_tag>{});
        }

        EHandleEventResult handleEvent(SubscriberID id, EHandleEvent event)
        {
            auto client_it = m_active_subscriptions.find(id);
            if(m_active_subscriptions.end() == client_it)
            {
                return EHandleEventResult::E_RESULT_INVALID_REFERENCE;
            }

            if (EHandleEvent::E_HANDLE_EVENT_OUT == event)
            {
                handleConnectionSuccess(client_it);
                return EHandleEventResult::E_RESULT_SUCCESS;
            }

            handleConnectionFailure(client_it, event);
            return EHandleEventResult::E_RESULT_FAILURE;
        }

     protected:
        template<typename Endpoint>
        bool subscribe(const CompletionCallback & cb, std::unique_ptr<Endpoint> p_endpoint)
        {
            using ConcreteWrapper = ClientDynamicTransportEndpointWrapper<Endpoint>;
            events_array subscribed_events = getArray<EHandleEvent::E_HANDLE_EVENT_OUT>();
            auto handle = GenericDeviceAccess::getHandle(p_endpoint->getDevice());
            SubscriberID id = m_demultiplexer.subscribe(subscribed_events, handle, *this);

            if (NULL_SUBSCRIBER_ID != id)
            {
                auto p_endpoint_wrapper = meta::traits::static_cast_unique_ptr<ConcreteWrapper, IClientTransportEndpoint>(
                    std::make_unique<ConcreteWrapper>(std::move(p_endpoint)));

                m_active_subscriptions[id] = ConnectorClientSubscriber{std::move(p_endpoint_wrapper), cb};
                return true;
            }

            std::cout << "Connector::subscribe() failure subscribing to demultiplexer.\n";
            return false;
        }

        template<typename Endpoint, typename DeviceAddress>
        bool connect(const CompletionCallback & cb, const DeviceAddress & addr, std::unique_ptr<Endpoint> p_endpoint, non_fifo_device_tag)
        {
            bool non_blocking{true};
            if (p_endpoint->connect(addr, non_blocking))
            {
                std::cout << "Connector::connect(non_fifo_device_tag) device connection done with success\n";
                return subscribe(cb, std::move(p_endpoint));
            }
            return false;
        }

        template<typename Endpoint, typename DeviceAddress>
        bool connect(const CompletionCallback & cb, const DeviceAddress & addr, std::unique_ptr<Endpoint> p_endpoint, fifo_device_tag)
        {
            using ConcreteWrapper = ClientDynamicTransportEndpointWrapper<Endpoint>;
            bool non_blocking{true};
            bool ret{false};
            std::unique_ptr<IClientTransportEndpoint> p_endpoint_wrapper{nullptr};

            if (p_endpoint->connect(addr, non_blocking))
            {
                std::cout << "Connector::connect(fifo_device_tag) device connection done with success\n";
                p_endpoint_wrapper = meta::traits::static_cast_unique_ptr<ConcreteWrapper, IClientTransportEndpoint>(
                                     std::make_unique<ConcreteWrapper>(std::move(p_endpoint)));
                
                postConnectionAction(p_endpoint_wrapper);
                ret = true;
            }

            cb(std::move(p_endpoint_wrapper));
            return ret;
        }

     private:
        void handleConnectionSuccess(SubscriberIter client_it)
        {
            std::cout << "Connector::handleConnectionSuccess\n";
            m_demultiplexer.unsubscribe(client_it->first);

            postConnectionAction(client_it->second.p_wrapped_endpoint);
            completeClientService(client_it);
        }

        void handleConnectionFailure(SubscriberIter client_it, EHandleEvent event)
        {
            std::cout << "Connector::handleConnectionFailure\n";
            if (enum_flag(event) & (enum_flag(EHandleEvent::E_HANDLE_EVENT_ERR) |
                                    enum_flag(EHandleEvent::E_HANDLE_EVENT_HUP)))
            {
                client_it->second.p_wrapped_endpoint.reset(nullptr);
                m_demultiplexer.unsubscribe(client_it->first);
                completeClientService(client_it);
            }
        }

        void postConnectionAction(std::unique_ptr<IClientTransportEndpoint> & p_endpoint_wrapper)
        {
            p_endpoint_wrapper->onConnected();
            events_array events = getArray<EHandleEvent::E_HANDLE_EVENT_IN>();
            p_endpoint_wrapper->listenerSubscribe(events);
        }

        void completeClientService(SubscriberIter client_it)
        {
            auto & client_sub = client_it->second;
            client_sub.completion_callback(std::move(client_sub.p_wrapped_endpoint));
            m_active_subscriptions.erase(client_it);
        }

    private:
        static constexpr SubscriberID NULL_SUBSCRIBER_ID{Demultiplexer::NULL_SUBSCRIBER_ID};

   private:
       SubscribersMap m_active_subscriptions;
       Demultiplexer & m_demultiplexer;
    };
}
#endif
