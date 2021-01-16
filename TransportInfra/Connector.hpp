#ifndef CONNECTOR_HPP
#define CONNECTOR_HPP
#include <functional>
#include <vector>
#include <memory>

#include "Reactor/SubscriberInfo.hpp"
#include "Reactor/EventTypes.h"
#include "Devices/DeviceFactory.hpp"
#include "Policies/ConnectionPolicy.hpp"
#include "Traits/transport_traits.hpp"
#include "PoliciesHolder.hpp"
#include "TransportEndpoint.hpp"

namespace infra
{

    using CompletionCallback = std::function<void(std::unique_ptr<AbstractTransportEndpoint>)>;

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

        public:
            /*
            template<typename Device>
            void test()
            {
                Device dev;
                //auto h = dev.getHandle();
                auto h = GenericDeviceAccess::getHandle(dev);
            }
            */
        // can't use a static method as we need a class local cache
        // to manage subscriptions to the epoll/select mechanisms
        template<typename ResourceHandler,
                 typename ConnectionParams,
                 template<typename...> typename DeviceConnectionPolicy,
                 template<typename...> typename... DevicePolicies>
        void setup(const ConnectionParams &params, CompletionCallback & cb)
        {
            /*
             * TODO
             * Delegate this type identification to some trait class
             */
            /*
            using device_t = typename DeviceFactory<params.dev_type>::template device_type<ResourceHandler>;
            using handle_t = typename device_t::handle_type;
            using device_host_t = Host<device_t, DeviceConnectionPolicy, DevicePolicies...>;
            using policies_holder_t = typename ConnectionParams::transport_policies_pack;
            using transport_endpoint_t = typename policies_holder_t::template AssembledClientT<TransportEndpoint<device_host_t>>;

            using traits_generator = transport_traits<ResourceHandler, ConnectionParams,
                                                      DeviceConnectionPolicy, DevicePolicies...>;
            using handle_t = typename traits_generator::handle_t;
            using device_host_t = typename traits_generator::device_host_t;
            using transport_endpoint_t = typename traits_generator::transport_endpoint_t;

            transport_endpoint_t endpoint;
            endpoint.setDevice(DeviceFactory<params.dev_type>::template createDevice<ResourceHandler, 
                                                                  DeviceConnectionPolicy,
                                                                  DevicePolicies...>());

            device_host_t & device = endpoint.getDevice();
            */
            /* TODO
             *
             * call device.Connect async with DeviceAddress as param
             * get the handle from the Device and register it with the EventDemultiplexer
             * after the connection succeeds and it the Connector is notified
             * create the TransportEndpoint and wrap it over the device
             * connect the callbacks and register it to the EventDemultiplexer
             * call the CompletionCallback
             */
            /*
            if (!device.connect(params.addr, true))
            {
                return;
            }

            //handle_t handle = device.getHandle();
            handle_t handle = GenericDeviceAccess::getHandle(device);
            events_array subscribed_events = getArray<EHandleEvent::E_HANDLE_EVENT_IN>();

            if (subscriber_id id = m_demultiplexer.subscribe(subscribed_events, handle, *this); NULL_SUBSCRIBER_ID != id)
            {
               m_active_subscriptions.emplace_back(id);
            }
            */

            /*
             * TODO
             * construct a type that can hold type information
             * store all relevant type info in that class -> make it as a return type to this method
             * allocate transport_endpoint with unique_ptr and store it in a container using type erasure
             *
             * after handleEvent is called and we call the completion callback, the client code already
             * has the type info needed to cast the erased type in unique ptr
             *
             * delegate all this boilerplate to a crtp base class of the client class that handles
             * the connector interface
             */
        }

        template<typename ResourceHandler,
                 typename DevicePolicies,
                 typename TransportPolicies,
                 typename CompletionCallback,
                 typename DeviceAddress,
                 std::size_t DeviceTag >
        bool setup(const ConnectionParameters<DeviceTag, DeviceAddress> & conn_params, CompletionCallback cb)
        {
            using types_holder = transport_traits<DeviceTag, ResourceHandler, DevicePolicies, TransportPolicies>;
            using device_t = typename types_holder::device_t;
            using handle_t = typename types_holder::handle_t;
            using device_host_t = typename types_holder::device_host_t;
            using transport_endpoint_t = typename types_holder::transport_endpoint_t;

            transport_endpoint_t p_endpoint = std::make_unique<transport_endpoint_t>();
            p_endpoint->setDevice(DeviceFactory<DeviceTag>::template createDevice<ResourceHandler, DevicePolicies>());
            device_host_t & device = p_endpoint->getDevice();
            bool async_mode{true};

            // for test only return before connect:
            cb(std::move(p_endpoint));


            if (!device.connect(conn_params.address, async_mode)){
                return false;
            }

            handle_t handle = GenericDeviceAccess::getHandle(device);
            events_array subscribed_events = getArray<EHandleEvent::E_HANDLE_EVENT_IN>();

            if (subscriber_id id = m_demultiplexer.subscribe(subscribed_events, handle, *this);
                 NULL_SUBSCRIBER_ID != id){
                m_active_subscriptions.emplace_back(id);
            }
            else return false;


            return true;
        }

        /*
         * TODO
         * When the connect succeeds -> call the Client completion callback with the Transport Endpoint as param
         */
        EHandleEventResult handleEvent(EHandleEvent event)
        {
            (void) event;
            return EHandleEventResult::E_RESULT_SUCCESS;
        }

   private:
        std::vector<subscriber_id> m_active_subscriptions; 
        Demultiplexer & m_demultiplexer;
    };
}
#endif
