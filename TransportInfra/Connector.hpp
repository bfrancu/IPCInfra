#ifndef CONNECTOR_HPP
#define CONNECTOR_HPP
#include <functional>
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

namespace infra
{

    using CompletionCallback = std::function<void()>;

    /* TODO
     *
     * Move connection logic to alternative Sync/Async Connection Policy
     * Make the Async Conn policy thread safe
     */

    template<typename Demultiplexer, typename Enable = void>
    class Connector
    {

     public:
            Connector(Demultiplexer & eventDemultiplexer) :
                m_demultiplexer(eventDemultiplexer)
        {}

        struct ConnectorClientSubscriber;
        using CompletionCallback = void(*)(std::unique_ptr<ITransportEndpoint>&&);
        using SubscriberID = typename Demultiplexer::SubscriberID;
        using SubscribersMap = std::unordered_map<SubscriberID, ConnectorClientSubscriber>;
        using SubscriberIter = typename SubscribersMap::iterator;
        using Listener = Demultiplexer;

        struct ConnectorClientSubscriber
        {
            ConnectorClientSubscriber() = default;
            ConnectorClientSubscriber(std::unique_ptr<ITransportEndpoint> && p_endpoint, CompletionCallback cb) :
                p_wrapped_endpoint{std::move(p_endpoint)},
                completion_callback{cb}
            {}

            std::unique_ptr<ITransportEndpoint> p_wrapped_endpoint{nullptr};
            CompletionCallback completion_callback{nullptr};
        };

        template<typename TransportTraits, typename... DeviceConstructorArgs>
        bool setup(const typename TransportTraits::device_address_t & addr,
                             const CompletionCallback & cb, DeviceConstructorArgs&&... args)
        {
            std::cout << "Connector::setup() connecting device; address: " << addr << "\n";
            using resource_handler_t = typename TransportTraits::resource_handler_t;
            using device_policies_t = typename TransportTraits::device_policies_t;
            using handle_t = typename TransportTraits::handle_t;
            using device_host_t = typename TransportTraits::device_host_t;
            using transport_endpoint_t = typename TransportTraits::transport_endpoint_t;
            using ConcreteWrapper = DynamicTransportEndpointWrapper<transport_endpoint_t>;

            std::cout <<"Connector::setup() creating new endpoint\n";

            auto p_endpoint = std::make_unique<transport_endpoint_t>(m_demultiplexer);

            std::cout <<"Connector::setup() setting new device in the endpoint\n";

            p_endpoint->setDevice(DeviceFactory<TransportTraits::device_tag>::template 
                        createDevice<resource_handler_t, device_policies_t>(std::forward<DeviceConstructorArgs>(args)...));

            std::cout << "Connector::setup() Device was set\n";
            device_host_t & device = p_endpoint->getDevice();
            bool non_blocking{true};

            handle_t handle = GenericDeviceAccess::getHandle(device);
            events_array subscribed_events = getArray<EHandleEvent::E_HANDLE_EVENT_OUT>();

            if (SubscriberID id = m_demultiplexer.subscribe(subscribed_events, handle, *this);
                    Demultiplexer::NULL_SUBSCRIBER_ID != id){

                std::cout << "Connector::setup() subscribed to demultiplexer.\n";
                if (device.connect(addr, non_blocking)){
                    auto p_endpoint_wrapper = meta::traits::static_cast_unique_ptr<ConcreteWrapper, ITransportEndpoint>(
                                              std::make_unique<ConcreteWrapper>(std::move(p_endpoint)));

                    m_active_subscriptions[id] = ConnectorClientSubscriber{std::move(p_endpoint_wrapper), cb};
                    return true;
                    std::cout << "Connector::setup() device connection done with success\n";
                }
                else
                {
                    // should unsubscribe from demultiplexer in case of connection failure
                }
                std::cout << "Connector::setup() device connection done with failure\n";
            }
            else
            {
                std::cout << "Connector::setup() failure subscribing to demultiplexer.\n";
            }

            return false;
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
        void handleConnectionSuccess(SubscriberIter client_it)
        {
            events_array events = getArray<EHandleEvent::E_HANDLE_EVENT_IN>();
            client_it->second.p_wrapped_endpoint->listenerSubscribe(events);
            completeClientService(client_it);
        }

        void handleConnectionFailure(SubscriberIter client_it, EHandleEvent event)
        {
            if (enum_flag(event) & (enum_flag(EHandleEvent::E_HANDLE_EVENT_ERR) |
                                    enum_flag(EHandleEvent::E_HANDLE_EVENT_HUP)))
            {
                client_it->second.p_wrapped_endpoint.reset(nullptr);
                completeClientService(client_it);
            }
        }

     private:
        void completeClientService(SubscriberIter client_it)
        {
            auto & client_sub = client_it->second;
            client_sub.completion_callback(std::move(client_sub.p_wrapped_endpoint));
            m_active_subscriptions.erase(client_it);
            m_demultiplexer.unsubscribe(client_it->first);
        }

   private:
       SubscribersMap m_active_subscriptions;
       Demultiplexer & m_demultiplexer;
    };
}
#endif
