#ifndef EPOLLDEMUX_H
#define EPOLLDEMUX_H

#include <stdint.h>

#include "EventTypes.h"

namespace infra
{

class EpollDemux
{
public:
    EpollDemux();

public:
    static uint32_t getEventsMask(const events_array & events);
    static events_array getEventsFromMask(uint32_t mask);

};

} //infra
#endif // EPOLLDEMUX_H
