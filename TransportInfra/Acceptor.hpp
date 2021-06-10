#ifndef ACCEPTOR_HPP
#define ACCEPTOR_HPP

#include "EndpointConnectionInitialiserBase.hpp"
#include "TransportEndpoint.hpp"
#include "Traits/storage_traits.hpp"

namespace infra
{

template<typename Demultiplexer, typename Enable = void>
class Acceptor : public EndpointConnectionInitialiserBase<Acceptor<Demultiplexer>, Demultiplexer, IServerTransportEndpoint>
{
    using Base = EndpointConnectionInitialiserBase<Acceptor<Demultiplexer>, Demultiplexer, IServerTransportEndpoint>;
    using CompletionCallback = typename Base::CompletionCallback;
    using SubscriberID = typename Base::SubscriberID;
    using SubscriberIter = typename Base::SubscriberIter;

    friend class EndpointConnectionInitialiserBase<Acceptor<Demultiplexer>, Demultiplexer, IServerTransportEndpoint>;

public:
    Acceptor(Demultiplexer & eventDemultplexer) :
        Base{eventDemultplexer}
    {}

    Acceptor(const Acceptor &) = delete;
    Acceptor & operator=(const Acceptor &) = delete;

protected:
    template<typename TransportTraits, typename... DeviceConstructorArgs,
             typename = std::enable_if_t<std::negation_v<traits::is_endpoint_storage<typename TransportTraits::endpoint_storage_t>>>>
    static bool setupImpl(...) { return false; }

    template<typename TransportTraits, typename... DeviceConstructorArgs>
    std::enable_if_t<traits::is_endpoint_storage_v<typename TransportTraits::endpoint_storage_t>, bool>
    setupImpl(const typename TransportTraits::device_address_t & addr, const CompletionCallback & cb, DeviceConstructorArgs&&... args)
    {
        std::cout << "Acceptor::setup() setting up device; address: " << addr << "\n";

        auto p_endpoint = Base::template factorEndpoint<TransportTraits>(Base::getListener(), std::forward<DeviceConstructorArgs>(args)...) ;

        if (p_endpoint)
        {
            return listenForConnections(cb, addr, std::move(p_endpoint));
        }

        return false;
    }

    template<typename Endpoint, typename DeviceAddress>
    bool listenForConnections(const CompletionCallback & cb, const DeviceAddress & addr, std::unique_ptr<Endpoint> p_endpoint)
    {
        std::cout << "Acceptor::listenForConnections\n";
        bool reusable_address{true};
        if (p_endpoint->bind(addr, reusable_address) && p_endpoint->listen())
        {
            using ConcreteWrapper = ServerDynamicTransportEndpointWrapper<Endpoint>;
            events_array subscribed_events = getArray<EHandleEvent::E_HANDLE_EVENT_IN>();
            return Base::template subscribe<ConcreteWrapper>(cb, std::move(p_endpoint), subscribed_events);
        }
        return false;
    }

    EHandleEventResult handleEventImpl(SubscriberIter, EHandleEvent event)
    {
        if (EHandleEvent::E_HANDLE_EVENT_IN == event)
        {
            return EHandleEventResult::E_RESULT_SUCCESS;
        }

        if (EHandleEvent::E_HANDLE_EVENT_ERR == event)
        {
            return EHandleEventResult::E_RESULT_FAILURE;
        }

        return EHandleEventResult::E_RESULT_DEFAULT;
    }

    void handleConnectionSuccess(SubscriberIter client_it)
    {
        std::cout << "Acceptor::handleConnectionSuccess\n";
        Base::handleConnectionSuccess(client_it);
    }

    void handleConnectionFailure(SubscriberIter client_it)
    {
        std::cout << "Acceptor::handleConnectionFailure\n";
        Base::handleConnectionFailure(client_it);
    }

    bool postConnectionAction(std::unique_ptr<IServerTransportEndpoint> & p_endpoint_wrapper)
    {
        std::cout << "Acceptor::postConnectionAction\n";
        bool non_blocking{true};
        if (p_endpoint_wrapper->accept(non_blocking))
        {
            events_array events = getArray<EHandleEvent::E_HANDLE_EVENT_IN>();
            p_endpoint_wrapper->listenerSubscribe(events);
            return true;
        }
        return false;
    }
};

}//infra
#endif // ACCEPTOR_HPP
