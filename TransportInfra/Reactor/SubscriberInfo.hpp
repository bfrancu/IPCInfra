#ifndef SUBSCRIBERINFO_HPP
#define SUBSCRIBERINFO_HPP

#include <stdint.h>

#include"Traits/handler_traits.hpp"
#include "EventHandlerSubscriber.h"

namespace infra
{

using subscriber_id = unsigned long;
inline constexpr subscriber_id NULL_SUBSCRIBER_ID{0};

template<typename handle_type>
struct SubscriberInfo
{
    SubscriberInfo(uint32_t events_mask, subscriber_id id, AbstractEventHandler & ev_handler) :
        registered_to_monitor{false},
        expired{false},
        handle{default_value<handle_type>::value},
        subscribed_events_mask{events_mask},
        id{id},
        event_handler{ev_handler}
    {}

    bool registered_to_monitor;
    bool expired;
    handle_type handle;
    uint32_t subscribed_events_mask;
    subscriber_id id;
    AbstractEventHandler & event_handler;
};


template <typename handle_type>
struct EventNotification
{
    handle_type handle;
    uint32_t notified_events_mask;
};

} // infra

#endif // SUBSCRIBERINFO_HPP
