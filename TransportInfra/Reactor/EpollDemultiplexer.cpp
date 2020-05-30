#include <sys/epoll.h>
#include <algorithm>

#include "EpollDemultiplexer.h"
#include "sys_call_eval.h"

namespace
{
using namespace infra;

constexpr unsigned int EPOLL_SIZE{15};
constexpr unsigned int MAX_EPOLL_EVENTS{5};
constexpr unsigned int EVENTS_LIMITS_NO{2};
const int EPOLL_WAIT_TIMEOUT_MS{5000};

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

namespace demux
{

EpollDemultiplexer::EpollDemultiplexer() :
    m_mutex{},
    m_epoll_fd{-1},
    m_wait_timeout_ms{EPOLL_WAIT_TIMEOUT_MS}
{
    m_epoll_fd = ::epoll_create(EPOLL_SIZE);
}

EpollDemultiplexer::~EpollDemultiplexer()
{}

uint32_t EpollDemultiplexer::getEventsMask(const events_array &events)
{
    uint32_t ret{0};
    std::for_each(events.begin(), events.end(), [&ret](const auto & event){
        ret |= handleEventToEpollFlag[static_cast<size_t>(event)].second;
    });
    return ret;

}

events_array EpollDemultiplexer::getEventsFromMask(uint32_t mask)
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

void EpollDemultiplexer::monitorWaitThread()
{

}

bool EpollDemultiplexer::registerImpl(int descriptor, subscriber_id id, uint32_t events_mask)
{
    epoll_event subscription_event;
    subscription_event.events = events_mask;
    subscription_event.data.u64 = id;
    return sys_call_noerr_eval(::epoll_ctl, m_epoll_fd, EPOLL_CTL_ADD, descriptor, &subscription_event);
}

bool EpollDemultiplexer::unregisterImpl(int descriptor)
{
    return sys_call_noerr_eval(::epoll_ctl, m_epoll_fd, EPOLL_CTL_DEL, descriptor, nullptr);
}

} // demux
} // infra
