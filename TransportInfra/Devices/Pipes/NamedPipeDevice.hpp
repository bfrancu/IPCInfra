#ifndef NAMEDPIPEDEVICE_H
#define NAMEDPIPEDEVICE_H
#include <fcntl.h>

#include <string>
#include <iostream>

#include "Traits/handler_traits.hpp"
#include "sys_call_eval.h"
#include "FileStatusFlags.h"
#include "FileIODefinitions.h"
#include "utilities.hpp"

namespace infra
{

template<typename ResourceHandler, typename = void>
class NamedPipeDevice{};

template<typename ResourceHandler>
class ReadingNamedPipeDevice : public NamedPipeDevice<ResourceHandler>
{
public:
    using io_profile = read_only_profile;    

    using NamedPipeDevice<ResourceHandler>::NamedPipeDevice;

    bool open(bool non_blocking = false){
        using R = ResourceHandler;
        return NamedPipeDevice<R>::template open<ReadingNamedPipeDevice<R>>(NamedPipeDevice<R>::m_fifo_pathname, non_blocking);
    }

    bool open(const std::string & pathname, bool non_blocking = false){
        using R = ResourceHandler;
        return NamedPipeDevice<R>::template open<ReadingNamedPipeDevice<R>>(pathname, non_blocking);
    }
};

template<typename ResourceHandler>
class WritingNamedPipeDevice : public NamedPipeDevice<ResourceHandler>
{
public:
    using io_profile = write_only_profile;

    using NamedPipeDevice<ResourceHandler>::NamedPipeDevice;

    bool open(bool non_blocking = false){
        using R = ResourceHandler;
        std::cout << "WritingNamedPipeDevice<T>::open(bool) non blocking: " << std::boolalpha << non_blocking << "\n";
        return NamedPipeDevice<R>::template open<WritingNamedPipeDevice<R>>(NamedPipeDevice<R>::m_fifo_pathname, non_blocking);
    }

    bool open(const std::string & pathname, bool non_blocking = false){
        using R = ResourceHandler;
        return NamedPipeDevice<R>::template open<WritingNamedPipeDevice<R>>(pathname, non_blocking);
    }
};

template<typename ResourceHandler>
class NamedPipeDevice<ResourceHandler, std::enable_if_t<HasUnixHandleTypeT<ResourceHandler>::value>>
{

public:
    using handle_type = typename handler_traits<ResourceHandler>::handle_type;
    using platform    = typename handler_traits<ResourceHandler>::platform;

public:
    NamedPipeDevice() {}
    NamedPipeDevice(std::string pathname) :
        m_fifo_pathname{std::move(pathname)}
    {}

public:
    bool isOpen() const { return m_resource_handler.open(); }

    void setNonBlocking(bool non_blocking){
        io::FileStatusFlags flags{m_resource_handler.getHandle()};
        if (non_blocking != flags.nonBlock()){
            flags.setNonBlock(non_blocking);
        }
    }

    bool nonBlocking() const {
        io::FileStatusFlags flags{m_resource_handler.getHandle()};
        return flags.nonBlock();
    }

    std::string pathname() const { return m_fifo_pathname; }

protected:
    /*bool open(bool non_blocking = false){
        return open<NamedPipeDevice<ResourceHandler>>(m_fifo_pathname, non_blocking);
    }*/

    template<typename P>
    static bool open(...){
        std::cout << "open ...\n";
        return false;
    }

    template<typename P, typename = typename P::io_profile>
    bool open(const std::string & pathname, bool non_blocking = false){
        auto access_mode{ std::is_same_v<
                    typename P::io_profile,
                    read_only_profile> ? io::EAccessMode::E_READ_ONLY : io::EAccessMode::E_WRITE_ONLY };

        return openPipe(pathname, access_mode, non_blocking);
    }

    /*
    bool openRead(const std::string & pathname, bool non_blocking = false){
        return openPipe(pathname, io::EAccessMode::E_READ_ONLY, non_blocking);
    }

    bool openRead(bool non_blocking = false){
        return openPipe(m_fifo_pathname, io::EAccessMode::E_READ_ONLY, non_blocking);
    }

    bool openWrite(const std::string & pathname, bool non_blocking = false){
        return openPipe(pathname, io::EAccessMode::E_WRITE_ONLY, non_blocking);
    }

    bool openWrite(bool non_blocking = false){
        return openPipe(m_fifo_pathname, io::EAccessMode::E_WRITE_ONLY, non_blocking);
    }*/

    bool openPipe(const std::string & pathname, io::EAccessMode access_mode, bool non_blocking){
        std::cout << "NamedPipeDevice<T>::openPipe non blocking: " << std::boolalpha << non_blocking
                  << " path: " << pathname << "\n";

        if (pathname.empty() || m_resource_handler.open()) return false;

        int open_flags = io::getAccessModeFlag(access_mode);
        std::cout << "NamedPipeDevice<T>::openPipe open file flags " << open_flags <<"\n";

        open_flags |= O_CLOEXEC;

        if (non_blocking) open_flags |= O_NONBLOCK;

        if (int fd{::open(pathname.c_str(), open_flags)}; -1 != fd){
            std::cout << "NamedPipeDevice<T>::openPipe open::\n";
            m_resource_handler.acquire(fd);
            if (pathname != m_fifo_pathname) m_fifo_pathname = pathname;
            return true;
        }
        else if (utils::to_underlying(io::EFileIOError::E_ERROR_NXIO) == errno){
            std::cerr << "opening writting endpoint for FIFO before the reading endpoint will fail with ENXIO\n";
        }
        else std::cerr << "NamedPipeDevice<T>::openPipe ::open failed with " << errno << "\n";


        return false;
    }

    handle_type getHandle() const { return m_resource_handler.getHandle(); }

protected:
    ResourceHandler m_resource_handler {};
    std::string m_fifo_pathname        {};

};

} //infra

#endif // NAMEDPIPEDEVICE_H
