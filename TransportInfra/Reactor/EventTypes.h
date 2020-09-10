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

}

#endif // EVENTTYPES_H


