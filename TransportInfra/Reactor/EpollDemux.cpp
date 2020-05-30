#include <sys/epoll.h>

#include <algorithm>

#include "EpollDemux.h"

namespace
{
using namespace infra;

constexpr unsigned int EVENTS_LIMITS_NO{2};
constexpr std::array<std::pair<EHandleEvent, uint32_t>, MAX_EVENTS_NO + EVENTS_LIMITS_NO>
handleEventToEpollFlag{
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_NULL, 0),
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_IN, EPOLLIN),
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_PRIO_IN, EPOLLPRI),
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_SHUTDOWN, EPOLLRDHUP),
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_OUT, EPOLLOUT),
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_ERR, EPOLLERR),
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_HUP, EPOLLHUP),
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_LAST, 0),
};
}


namespace infra
{


EpollDemux::EpollDemux()
{

}

uint32_t EpollDemux::getEventsMask(const events_array &events)
{
    uint32_t ret{0};
    std::for_each(events.begin(), events.end(), [&ret](const auto & event){
        ret |= handleEventToEpollFlag[static_cast<size_t>(event)].second;
    });
    return ret;
}

infra::events_array EpollDemux::getEventsFromMask(uint32_t mask)
{
    infra::events_array ret_arr;
    ret_arr.fill(EHandleEvent::E_HANDLE_EVENT_NULL);
    std::for_each(handleEventToEpollFlag.begin() + 1, handleEventToEpollFlag.end() -1,
                  [=,&ret_arr] (const auto & pair){
          // we add it at the previous index because the events_array does not contain the limits
          // EVENT_NULL and EVENT_LAST and is 2 less in size
          if (mask & pair.second){ ret_arr[static_cast<size_t>(pair.first)-1] = pair.first; }
    });
    return ret_arr;
}

} //infra
