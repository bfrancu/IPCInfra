#include <algorithm>
#include <unordered_map>

#include "EpollDemultiplexer.h"
#include "sys_call_eval.h"



namespace infra
{

namespace demux
{

namespace
{

using namespace sys;
constexpr unsigned int EVENTS_LIMITS_NO{2};

constexpr std::array<uint32_t, MAX_EVENTS_NO + EVENTS_LIMITS_NO>
handleEventToEpollFlag{
   0,
   EPOLLIN,
   EPOLLPRI,
   EPOLLOUT,
   EPOLLERR,
   EPOLLHUP,
   EPOLLRDHUP,
   0
};

/*
constexpr std::array<std::pair<EHandleEvent, uint32_t>, MAX_EVENTS_NO + EVENTS_LIMITS_NO>
handleEventToEpollFlag1{
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_NULL, 0),
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_IN, EPOLLIN),
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_PRIO_IN, EPOLLPRI),
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_OUT, EPOLLOUT),
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_ERR, EPOLLERR),
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_HUP, EPOLLHUP),
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_SHUTDOWN, EPOLLRDHUP),
    std::make_pair(EHandleEvent::E_HANDLE_EVENT_LAST, 0)
};
*/

static std::unordered_map<EHandleEvent, uint32_t>
handleEventToEpollFlag2{
   {EHandleEvent::E_HANDLE_EVENT_NULL, 0},
   {EHandleEvent::E_HANDLE_EVENT_IN, EPOLLIN},
   {EHandleEvent::E_HANDLE_EVENT_PRIO_IN, EPOLLPRI},
   {EHandleEvent::E_HANDLE_EVENT_SHUTDOWN, EPOLLRDHUP},
   {EHandleEvent::E_HANDLE_EVENT_OUT, EPOLLOUT},
   {EHandleEvent::E_HANDLE_EVENT_ERR, EPOLLERR},
   {EHandleEvent::E_HANDLE_EVENT_HUP, EPOLLHUP},
   {EHandleEvent::E_HANDLE_EVENT_LAST, 0}
};
}

uint32_t getEventsMask(const events_array &events)
{
    uint32_t ret{0};
    std::for_each(events.begin(), events.end(), [&ret](const auto & event){
        if (EHandleEvent::E_HANDLE_EVENT_NULL != event){
            auto epollEvent = handleEventToEpollFlag[static_cast<unsigned>(event)];
            ret |= epollEvent;

            /*
            std::cout << "demux::getEventsMask() EHandleEvent : " << static_cast<uint32_t>(event)
                      << "; epollevent: " << epollEvent
                      << "; mask: " << ret << "\n";
                      */
        }
    });
    return ret;

}

events_array getEventsFromMask(uint32_t mask)
{
    infra::events_array ret_arr;
    ret_arr.fill(EHandleEvent::E_HANDLE_EVENT_NULL);
    for (unsigned index = static_cast<unsigned>(EHandleEvent::E_HANDLE_EVENT_IN); 
        index < static_cast<unsigned>(EHandleEvent::E_HANDLE_EVENT_LAST); ++index)
    {
        if (mask & handleEventToEpollFlag[index])
        {
            ret_arr[index] = static_cast<EHandleEvent>(index);
        }
    }
    /*
    std::for_each(handleEventToEpollFlag.begin() + 1, handleEventToEpollFlag.end() -1,
                  [=,&ret_arr] (const auto item){
          // we add it at the previous index because the events_array does not contain the limits
          // EVENT_NULL and EVENT_LAST and is 2 less in size
          if (mask & pair.second){ ret_arr[static_cast<size_t>(pair.first)-1] = pair.first; }
    });
    */
    return ret_arr;
}

} // demux
} // infra


