#ifndef EVENTHANDLERSUBSCRIBER_H
#define EVENTHANDLERSUBSCRIBER_H

#include "EventTypes.h"

namespace infra
{

struct AbstractEventHandler
{
    virtual EHandleEventResult handleEvent(EHandleEvent event) = 0;
};

template <typename T>
struct ConcreteEventHandler : AbstractEventHandler
{
     T* p_handler;

     explicit ConcreteEventHandler(T & ev_handler) :
         p_handler(&ev_handler)
     {}

     EHandleEventResult handleEvent(EHandleEvent event) override{
         if (p_handler) return p_handler->handleEvent(event);
         return EHandleEventResult::E_RESULT_INVALID_REFERENCE;
     }
};

}  //infra
#endif // EVENTHANDLERSUBSCRIBER_H
