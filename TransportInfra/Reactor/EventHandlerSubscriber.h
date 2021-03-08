#ifndef EVENTHANDLERSUBSCRIBER_H
#define EVENTHANDLERSUBSCRIBER_H

#include "EventTypes.h"

namespace infra
{

using subscriber_id = unsigned long;

struct AbstractEventHandler
{
    virtual EHandleEventResult handleEvent(subscriber_id id, EHandleEvent event) = 0;
};

template <typename T>
struct ConcreteEventHandler : AbstractEventHandler
{
     T* p_handler;

     explicit ConcreteEventHandler(T * p_ev_handler) :
         p_handler(p_ev_handler)
     {}

     EHandleEventResult handleEvent(subscriber_id id, EHandleEvent event) override{
         if (p_handler) return p_handler->handleEvent(id, event);
         return EHandleEventResult::E_RESULT_INVALID_REFERENCE;
     }
};

}  //infra
#endif // EVENTHANDLERSUBSCRIBER_H


