#ifndef FILEIODEFINITIONS_H
#define FILEIODEFINITIONS_H

#include <typeinfo>
#include <type_traits>

namespace infra
{

namespace io
{

using file_descriptor_index = int;

enum class EAccessMode : int
{
    E_UNDEFINED = -1,
    E_READ_ONLY,
    E_WRITE_ONLY,
    E_READ_WRITE,
    E_APPEND
};

enum EOpenFileFlag : int
{
    E_CREAT      = 0100,
    E_EXCL       = 0200,
    E_DIRECTORY  = 0200000,
    E_NOATIME    = 01000000,
    E_DIRECT     = 040000,
    E_TRUNC      = 01000,
    E_APPEND     = 02000,
    E_ASYNC      = 020000,
    E_SYNC       = 04010000,
    E_NONBLOCK   = 04000
};

enum class EFileType : int
{
    E_UNDEFINED = -1,
    E_REGULAR,
    E_DIRECTORY,
    E_CHARACTER_DEVICE,
    E_BLOCK_DEVICE,
    E_LOCAL_SOCKET,
    E_NAMED_PIPE,
    E_SYMBOLIC_LINK
};

enum EFilePermission : int
{
    E_USER_READ = 0,
    E_USER_WRITE,
    E_USER_EXECUTE,
    E_GROUP_READ,
    E_GROUP_WRITE,
    E_GROUP_EXECUTE,
    E_OTHER_READ,
    E_OTHER_WRITE,
    E_OTHER_EXECUTE,
    E_LAST
};

enum class ESeekWhence : int
{
    /*Do not change these values. They should match the C WHENCE Defines in unistd.h*/
    E_SEEK_SET = 0,  // From the start of the file
    E_SEEK_CUR = 1,  // From the current position
    E_SEEK_END = 2,  // From the end of file
};

enum class EFileIOError : int
{
    E_ERROR_UNKNOWN  = -1,
    E_ERROR_FIRST    = 0,
    E_ERROR_PERM     = 1,      /* Operation not permitted */
    E_ERROR_NOENT    = 2,      /* No such file or directory */
    E_ERROR_SRCH     = 3,      /* No such process */
    E_ERROR_INTR     = 4,      /* Interrupted system call */
    E_ERROR_IO       = 5,      /* I/O error */
    E_ERROR_NXIO     = 6,      /* No such device or address */
    E_ERROR_2BIG     = 7,      /* Argument list too long */
    E_ERROR_NOEXEC   = 8,      /* Exec format error */
    E_ERROR_BADF     = 9,      /* Bad file number */
    E_ERROR_CHILD    = 10,     /* No child processes */
    E_ERROR_AGAIN    = 11,     /* Try again */
    E_ERROR_NOMEM    = 12,     /* Out of memory */
    E_ERROR_ACCES    = 13,     /* Permission denied */
    E_ERROR_FAULT    = 14,     /* Bad address */
    E_ERROR_NOTBLK   = 15,     /* Block device required */
    E_ERROR_BUSY     = 16,     /* Device or resource busy */
    E_ERROR_EEXIST   = 17,     /* File exists */
    E_ERROR_XDEV     = 18,     /* Cross-device link */
    E_ERROR_NODEV    = 19,     /* No such device */
    E_ERROR_NOTDIR   = 20,     /* Not a directory */
    E_ERROR_ISDIR    = 21,     /* Is a directory */
    E_ERROR_INVAL    = 22,     /* Invalid argument */
    E_ERROR_NFILE    = 23,     /* File table overflow */
    E_ERROR_MFILE    = 24,     /* Too many open files */
    E_ERROR_NOTTY    = 25,     /* Not a typewriter */
    E_ERROR_TXTBSY   = 26,     /* Text file busy */
    E_ERROR_EFBIG    = 27,     /* File too large */
    E_ERROR_NOSPC    = 28,     /* No space left on device */
    E_ERROR_SPIPE    = 29,     /* Illegal seek */
    E_ERROR_ROFS     = 30,     /* Read-only file system */
    E_ERROR_MLINK    = 31,     /* Too many links */
    E_ERROR_PIPE     = 32,     /* Broken pipe */
    E_ERROR_DOM      = 33,     /* Math argument out of domain of func */
    E_ERROR_RANGE    = 34,     /* Math result not representable */
    E_ERROR_LAST     = 35
};

//template <typename E>
//constexpr typename std::underlying_type<E>::type to_underlying(E e)
//{
//    return static_cast<typename std::underlying_type<E>::type>(e);
//}

} //io

} //infra
#endif // FILEIODEFINITIONS_H
