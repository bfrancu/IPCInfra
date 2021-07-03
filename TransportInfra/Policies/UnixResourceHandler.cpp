#include <fcntl.h> //open
#include <errno.h>  // errno
#include <unistd.h>
#include <iostream>

#include "UnixResourceHandler.h"
#include "FileStatusFlags.h"

namespace infra
{

UnixResourceHandler::UnixResourceHandler(UnixResourceHandler::handle_type handle) :
    m_handle{handle}
{
    std::cout << "UnixResourceHandler::UnixResourceHandler() handle: " << handle << "\n";
    m_open = !defaultHandle();
    io::FileStatusFlags flags{m_handle};
    /*
    std::cout << "UnixResourceHandler::UnixResourceHandler() is open: "
              << m_open << "; flags valid: " << flags.valid() << "\n";
    */
}

UnixResourceHandler::UnixResourceHandler(const UnixResourceHandler &other) :
    m_handle{DEFAULT_VALUE},
    m_open{other.m_open}
{
    std::cout << "UnixResourceHandler::UnixResourceHandler(const UnixResourceHandler & other) other.handle: " << other.m_handle << "\n";
    if (DEFAULT_VALUE != other.m_handle)
    {
        acquire(::dup(other.m_handle));
        std::cout << "UnixResourceHandler::UnixResourceHandler(const UnixResourceHandler & other) duplicated m_handle: " << m_handle << "\n";
    }
}

UnixResourceHandler::UnixResourceHandler(UnixResourceHandler &&other) noexcept:
    m_handle{other.m_handle},
    m_open{other.m_open}
{
    std::cout << "UnixResourceHandler::UnixResourceHandler(UnixResourceHandler && other) other.handle: " << other.m_handle << "\n";
    other.m_open = false;
    other.m_handle = DEFAULT_VALUE;
}

UnixResourceHandler &UnixResourceHandler::operator=(const UnixResourceHandler &other)
{
    std::cout << "UnixResourceHandler::operator=(const UnixResourceHandler & other) other.handle: " << other.m_handle << "\n";
    close();
    if (this != &other){
        acquire(::dup(other.m_handle));
        std::cout << "UnixResourceHandler::operator=(const UnixResourceHandler & other) duplicated m_handle: " << m_handle << "\n";
    }
    return *this;
}

UnixResourceHandler &UnixResourceHandler::operator=(UnixResourceHandler &&other) noexcept
{
    std::cout << "UnixResourceHandler::operator=(UnixResourceHandler && other) other.handle: " << other.m_handle << "\n";
    close();
    acquire(other.release());
    return *this;
}

UnixResourceHandler::~UnixResourceHandler()
{
    std::cout << "UnixResourceHandler::~UnixResourceHandler()\n";
    close();
}

void UnixResourceHandler::acquire(handle_type handle)
{
     if (!defaultHandle() && handle != m_handle)
     {
         close();
     }
     std::cout << "UnixResourceHandler::aquire() handle: " << handle << "\n";
     m_open = true;
     m_handle = handle;
}

bool UnixResourceHandler::validHandle() const
{
    return ::fcntl(m_handle, F_GETFD) != -1 || errno != EBADF;
}

bool UnixResourceHandler::open() const
{
    errno = 0;
    return (-1 != ::fcntl(m_handle, F_GETFD) || EBADF != errno);
}

bool UnixResourceHandler::close()
{
    std::cout << "UnixResourceHandler::close() handle: " << m_handle << "\n";
    if (defaultHandle()){
        return true;
    }

    if (int res = ::close(m_handle); -1 != res){
        m_open = false;
        return true;
    }
    return false;
}

bool UnixResourceHandler::defaultHandle() const
{
    return DEFAULT_VALUE == m_handle;
}

} // infra
