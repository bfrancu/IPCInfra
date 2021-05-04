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
    m_open = !defaultHandle();
    io::FileStatusFlags flags{m_handle};
    std::cout << "UnixResourceHandler::UnixResourceHandler() is open: "
              << m_open << "; flags valid: " << flags.valid() << "\n";
}

UnixResourceHandler::UnixResourceHandler(const UnixResourceHandler &other) :
    m_open{other.m_open}
{
    if (DEFAULT_VALUE != other.m_handle)
    {
        m_handle = ::dup(other.m_handle);
    }
}

UnixResourceHandler::UnixResourceHandler(UnixResourceHandler &&other) noexcept:
    m_handle{other.m_handle},
    m_open{other.m_open}
{
    other.m_open = false;
    other.m_handle = DEFAULT_VALUE;
}

UnixResourceHandler &UnixResourceHandler::operator=(const UnixResourceHandler &other)
{
    if (this != &other) acquire(::dup(other.m_handle));
    return *this;
}

UnixResourceHandler &UnixResourceHandler::operator=(UnixResourceHandler &&other) noexcept
{
    acquire(other.release());
    return *this;
}

UnixResourceHandler::~UnixResourceHandler()
{
    if (!defaultHandle()){
        close();
    }
}

void UnixResourceHandler::acquire(handle_type handle)
{
     if (!defaultHandle() && handle != m_handle)
     {
         close();
     }
     m_open = true;
     m_handle = handle;
}

bool UnixResourceHandler::validHandle() const
{
    return ::fcntl(m_handle, F_GETFD) != -1 || errno != EBADF;
}

void UnixResourceHandler::close()
{
    ::close(m_handle);
    m_open = false;
}

bool UnixResourceHandler::defaultHandle() const
{
    return DEFAULT_VALUE == m_handle;
}

} // infra
