#ifndef SUBSCRIBERINFO_HPP
#define SUBSCRIBERINFO_HPP

#include <stdint.h>

#include"Traits/handler_traits.hpp"
#include "EventHandlerSubscriber.h"

namespace infra
{

using subscriber_id = unsigned long;
inline constexpr subscriber_id NULL_SUBSCRIBER_ID{0};

struct DummyEventHandler : AbstractEventHandler
{
    EHandleEventResult handleEvent(EHandleEvent event) override{
        (void) event;
        return EHandleEventResult::E_RESULT_METHOD_NOT_IMPLEMENTED;
    }
};

template<typename handle_t>
struct SubscriberInfo
{
    SubscriberInfo() :
        registered_to_monitor{false},
        expired{false},
        handle{default_value<handle_t>::value},
        subscribed_events_mask{0},
        id{0},
        event_handler{dummy_handler}
    {}

    SubscriberInfo(uint32_t events_mask, subscriber_id id, AbstractEventHandler & ev_handler) :
        registered_to_monitor{false},
        expired{false},
        handle{default_value<handle_t>::value},
        subscribed_events_mask{events_mask},
        id{id},
        event_handler{ev_handler}
    {}

    bool registered_to_monitor;
    bool expired;
    handle_t handle;
    uint32_t subscribed_events_mask;
    subscriber_id id;
    AbstractEventHandler & event_handler;

private:
    inline static DummyEventHandler dummy_handler{};
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


