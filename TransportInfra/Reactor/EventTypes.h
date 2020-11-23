#ifndef EVENTTYPES_H
#define EVENTTYPES_H
#include <array>

namespace infra
{

enum class EHandleEventResult
{
    E_RESULT_METHOD_NOT_IMPLEMENTED = -4,
    E_RESULT_INVALID_REFERENCE = -3,
    E_RESULT_FAILURE = -2,
    E_RESULT_DEFAULT = -1,
    E_RESULT_SUCCESS = 0
};

enum class EHandleEvent
{
    E_HANDLE_EVENT_NULL = 0,
    E_HANDLE_EVENT_IN = 0x001,
    E_HANDLE_EVENT_PRIO_IN = 0x002,
    E_HANDLE_EVENT_OUT = 0x003,
    E_HANDLE_EVENT_ERR = 0x004,
    E_HANDLE_EVENT_HUP = 0x005,
    E_HANDLE_EVENT_SHUTDOWN = 0x006,
    E_HANDLE_EVENT_LAST
};

inline const unsigned MAX_EVENTS_NO = 6;
using events_array = std::array<EHandleEvent, MAX_EVENTS_NO>;

template<std::size_t index>
constexpr void fillArray(events_array & arr)
{
    (void) arr;
}

template<std::size_t index, EHandleEvent event, EHandleEvent... events>
constexpr void fillArray(events_array & arr)
{
   if constexpr(!(std::tuple_size<events_array>::value <= index))
   {
        std::get<index>(arr) = event;
        fillArray<index+1, events...>(arr);
   }
}

template<EHandleEvent... events>
constexpr events_array getArray()
{
    events_array arr{
        EHandleEvent::E_HANDLE_EVENT_NULL,
        EHandleEvent::E_HANDLE_EVENT_NULL,
        EHandleEvent::E_HANDLE_EVENT_NULL,
        EHandleEvent::E_HANDLE_EVENT_NULL,
        EHandleEvent::E_HANDLE_EVENT_NULL,
        EHandleEvent::E_HANDLE_EVENT_NULL
    };

    fillArray<0, events...>(arr);
    return arr;
};


}

#endif // EVENTTYPES_H


