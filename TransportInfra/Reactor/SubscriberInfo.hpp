#ifndef SUBSCRIBERINFO_HPP
#define SUBSCRIBERINFO_HPP

#include <stdint.h>
//#include <iostream>

#include "default_traits.hpp"
#include"Traits/handler_traits.hpp"
#include "EventHandlerSubscriber.h"

namespace infra
{

//constexpr subscriber_id NULL_SUBSCRIBER_ID{0};

/*
struct DummyEventHandler : AbstractEventHandler
{
    EHandleEventResult handleEvent(subscriber_id, EHandleEvent event) override{
        //std::cout << "DummyEventHandler::handleEvent()\n";
        (void) event;
        return EHandleEventResult::E_RESULT_METHOD_NOT_IMPLEMENTED;
    }
    AbstractEventHandler *clone() override { return nullptr; }
};*/

template<typename handle_t>
struct SubscriberContext
{
    SubscriberContext() :
        handle{meta::traits::default_value<handle_t>::value},
        subscribed_events_mask{0},
        id{0}
    {}

    SubscriberContext(uint32_t events_mask, subscriber_id id,
                      handle_t desc = meta::traits::default_value<handle_t>::value) :
        handle{desc},
        subscribed_events_mask{events_mask},
        id{id}
    {}

    SubscriberContext(const SubscriberContext & other) :
        registered_to_monitor(other.registered_to_monitor),
        expired(other.expired),
        handle(other.handle),
        subscribed_events_mask(other.subscribed_events_mask),
        id(other.id)
    {}

    SubscriberContext(SubscriberContext && other) :
        registered_to_monitor(other.registered_to_monitor),
        expired(other.expired),
        handle(other.handle),
        subscribed_events_mask(other.subscribed_events_mask),
        id(other.id)
    {}

    SubscriberContext & operator=(const SubscriberContext & other)
    {
        if (this == &other) return *this;
        registered_to_monitor = other.registered_to_monitor;
        expired = other.expired;
        handle = other.handle;
        subscribed_events_mask = other.subscribed_events_mask;
        id = other.id;
        return *this;
    }

    SubscriberContext & operator=(SubscriberContext && other)
    {
        registered_to_monitor = other.registered_to_monitor;
        expired = other.expired;
        handle = other.handle;
        subscribed_events_mask = other.subscribed_events_mask;
        id = other.id;
        return *this;
    }

    bool registered_to_monitor{false};
    bool expired{false};
    handle_t handle;
    uint32_t subscribed_events_mask;
    subscriber_id id;
};

template <typename handle_t>
struct EventNotification
{
    /*EventNotification() = default;
    EventNotification(handle_t hd, uint32_t mask, bool last_one = false) :
        handle{hd},
        notified_events_mask{mask},
        last_event{last_one}
    {}*/

    handle_t handle;
    uint32_t notified_events_mask;
    bool last_event {false};
};

} // infra

#endif // SUBSCRIBERINFO_HPP


