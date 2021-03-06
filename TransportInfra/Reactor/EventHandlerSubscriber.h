#ifndef EVENTHANDLERSUBSCRIBER_H
#define EVENTHANDLERSUBSCRIBER_H

#include <iostream>
#include <atomic>

#include "EventTypes.h"

namespace infra
{

using subscriber_id = unsigned long;

struct AbstractEventHandler
{
    virtual EHandleEventResult handleEvent(subscriber_id id, EHandleEvent event) = 0;
    virtual AbstractEventHandler *clone() = 0;
    virtual ~AbstractEventHandler() = default;
};

template <typename T>
struct ConcreteEventHandler : AbstractEventHandler
{
    T* p_handler{nullptr};

     explicit ConcreteEventHandler(T * p_ev_handler) :
         p_handler(p_ev_handler)
     {}

     EHandleEventResult handleEvent(subscriber_id id, EHandleEvent event) override{
         if (p_handler) return p_handler->handleEvent(id, event);
         return EHandleEventResult::E_RESULT_INVALID_REFERENCE;
     }

     AbstractEventHandler *clone() override {
         return new ConcreteEventHandler(p_handler);
     }
};

template <typename T>
struct EventHandlerBridge : AbstractEventHandler
{
    T* p_handler{nullptr};

     explicit EventHandlerBridge(T * p_ev_handler) :
         p_handler(p_ev_handler)
     {}

     EHandleEventResult handleEvent(subscriber_id id, EHandleEvent event) override{
         if (p_handler) return p_handler->handleEvent(id, event);
         return EHandleEventResult::E_RESULT_INVALID_REFERENCE;
     }

     AbstractEventHandler *clone() override {
         return new EventHandlerBridge(p_handler);
     }
};

struct EventHandlerWrapper
{
    EventHandlerWrapper() = default;

    template<typename T>
    EventHandlerWrapper(T *p_handler)
    {
        p_event_handler = new EventHandlerBridge<T>(p_handler);
    }

    EventHandlerWrapper(const EventHandlerWrapper & other)
    {
        std::cout << "EventHandlerWrapper::EventHandlerWrapper(const EventHandlerWrapper &)\n";
        if (other.p_event_handler)
        {
            p_event_handler = other.p_event_handler->clone();
        }
    }

    EventHandlerWrapper(EventHandlerWrapper && other)
    {
        std::cout << "EventHandlerWrapper::EventHandlerWrapper(EventHandlerWrapper &&)\n";
        if (other.p_event_handler)
        {
            p_event_handler = other.p_event_handler;
            other.p_event_handler = nullptr;
        }
    }

    EventHandlerWrapper & operator=(const EventHandlerWrapper & other)
    {
        std::cout << "EventHandlerWrapper::operator=(const EventHandlerWrapper &)\n";
        if (this == &other) return *this;
        if (other.p_event_handler) {
            p_event_handler = other.p_event_handler->clone();
        }
        return *this;
    }

    EventHandlerWrapper & operator=(EventHandlerWrapper && other)
    {
        std::cout << "EventHandlerWrapper::operator=(EventHandlerWrapper &&)\n";
        if (other.p_event_handler) {
            p_event_handler = other.p_event_handler;
            other.p_event_handler = nullptr;
        }
        return *this;
    }

    ~EventHandlerWrapper() {
        std::cout << "EventHandlerWrapper::~EventHandlerSubscriber()\n";
        if (p_event_handler) delete p_event_handler; }

    template<typename T>
    void assign(T *p_handler)
    {
        if (p_event_handler)
        {
            delete p_event_handler;
            p_event_handler = nullptr;
        }
        p_event_handler = new EventHandlerBridge<T>(p_handler);
    }

    EHandleEventResult handleEvent(subscriber_id id, EHandleEvent event)
    {
        if (p_event_handler) return p_event_handler->handleEvent(id, event);
        return EHandleEventResult::E_RESULT_INVALID_REFERENCE;
    }


    AbstractEventHandler *p_event_handler{nullptr};
    std::atomic<bool> in_progress{false};
};

}  //infra
#endif // EVENTHANDLERSUBSCRIBER_H
