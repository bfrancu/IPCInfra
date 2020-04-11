#ifndef SOCKETTYPES_H
#define SOCKETTYPES_H
#include <sys/socket.h>

#include "enum_flag.h"

namespace infra
{

namespace io
{

typedef enum ESocketType
{
    E_SOCK_ANY = 0,
    E_SOCK_STREAM = 1,
    E_SOCK_DGRAM = 2
}ESocketType;

typedef enum EAddressFamily
{
#ifdef AF_UNSPEC
    E_PF_UNSPEC = PF_UNSPEC,
#else
    E_PF_UNSPEC = 0,
#endif
#ifdef AF_UNIX
    E_PF_UNIX = PF_UNIX,
#else
    E_PF_UNIX = 1,
#endif
#ifdef AF_INET
    E_PF_INET = PF_INET,
#else
    E_PF_INET = 2,
#endif
#ifdef AF_INET6
    E_PF_INET6 = PF_INET6
#else
    E_PF_INET6 = 10
#endif
}EAddressFamily;

typedef enum ESocketState
{
    E_STATE_AVAILABLE = 0,
    E_STATE_BINDED,
    E_STATE_LISTENING,
    E_STATE_CONNECTING,
    E_STATE_CONNECTED,
    E_STATE_DISCONNECTED,
    E_STATE_SHUTDOWN,
    E_STATE_ERROR
}ESocketState;

typedef enum EShutdownHow
{
    E_SHUTDOWN_READ = 0,
    E_SHUTDOWN_WRITE,
    E_SHUTDOWN_READWRITE
} EShutdownHow;

/* pag 1259. 61.3 Socket-Specific I/O. To be used with send/recv socket specific calls*/
enum class ESocketIOFlag : int
{
    E_MSG_NO_FLAG = 0,
    E_MSG_DONTWAIT = 0x80,
    E_MSG_OOB = 0x01,
    E_MSG_PEEK = 0x02,   /* Peek at incoming messages.  */
    E_MSG_WAITALL = 0x100,  /* Wait for a full request.  */
    E_MSG_MORE = 0x8000, /* Sender will send more. */
    E_MSG_NOSIGNAL = 0x4000, /* Do not generate SIGPIPE.  */
};

using SocketIOFlags = enum_flag<io::ESocketIOFlag>;


enum class ESocketEvent
{
    E_SOCKET_INVALID = -1,
    E_SOCKET_IN = 0,
    E_SOCKET_PRIO_IN,
    E_SOCKET_SHUTDOWN,
    E_SOCKET_OUT,
    E_SOCKET_ERR,
    E_SOCKET_HUP,
    E_SOCKET_LAST
};

} //io
} //infra

#endif // SOCKETTYPES_H
