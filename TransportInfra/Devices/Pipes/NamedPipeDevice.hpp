#ifndef NAMEDPIPEDEVICE_H
#define NAMEDPIPEDEVICE_H
#include <fcntl.h>

#include <string>
#include <iostream>

#include "Traits/device_traits.hpp"
#include "sys_call_eval.h"
#include "FileStatusFlags.h"
#include "FileIODefinitions.h"
#include "utilities.hpp"
#include "LinuxUtils/LinuxIOUtilities.h"
#include "NamedPipeAddress.h"
#include "NamedPipeDeviceAccess.hpp"
#include "Devices/AccessibleHandleBase.h"

namespace infra
{

template<typename ResourceHandler, typename Address = NamedPipeAddress, typename = void>
class NamedPipeDevice{};

template<typename ResourceHandler, typename Address = NamedPipeAddress>
class ReadingNamedPipeDevice : public NamedPipeDevice<ResourceHandler, Address>
{
public:
    using io_profile = read_only_profile;    
    using address_type = typename NamedPipeDevice<ResourceHandler, Address>::address_type;
    using NamedPipeDevice<ResourceHandler>::NamedPipeDevice;

    bool open(bool non_blocking = false){
        using R = ResourceHandler;
        return NamedPipeDevice<R>::template openImpl<ReadingNamedPipeDevice<R>>(NamedPipeDevice<R>::m_fifo_pathname, non_blocking);
    }

    bool open(const NamedPipeAddress & addr, bool non_blocking = false){
        return open(addr.pathname, non_blocking);
    }

    bool open(const std::string & pathname, bool non_blocking = false){
        using R = ResourceHandler;
        return NamedPipeDevice<R>::template openImpl<ReadingNamedPipeDevice<R>>(pathname, non_blocking);
    }
};

template<typename ResourceHandler, typename Address = NamedPipeAddress>
class WritingNamedPipeDevice : public NamedPipeDevice<ResourceHandler, Address>
{
public:
    using io_profile = write_only_profile;
    using address_type = typename NamedPipeDevice<ResourceHandler, Address>::address_type;
    using NamedPipeDevice<ResourceHandler>::NamedPipeDevice;

    bool open(bool non_blocking = false){
        using R = ResourceHandler;
        std::cout << "WritingNamedPipeDevice<T>::open(bool) non blocking: " << std::boolalpha << non_blocking << "\n";
        return NamedPipeDevice<R>::template openImpl<WritingNamedPipeDevice<R>>(NamedPipeDevice<R>::m_fifo_pathname, non_blocking);
    }

    bool open(const NamedPipeAddress & addr, bool non_blocking = false){
        return open(addr.pathname, non_blocking);
    }

    bool open(const std::string & pathname, bool non_blocking = false){
        using R = ResourceHandler;
        return NamedPipeDevice<R>::template openImpl<WritingNamedPipeDevice<R>>(pathname, non_blocking);
    }
};

template<typename ResourceHandler, typename Address>
class NamedPipeDevice<ResourceHandler, Address, std::enable_if_t<HasUnixHandleTypeT<ResourceHandler>::value>>
    : public AccessibleHandleBase<NamedPipeDevice<ResourceHandler, Address>>
{
    friend class NamedPipeDeviceAccess;
    friend class GenericDeviceAccess;
    friend class AccessibleHandleBase<NamedPipeDevice<ResourceHandler, Address>>;
public:
    using handle_type  = typename handler_traits<ResourceHandler>::handle_type;
    using platform    = typename handler_traits<ResourceHandler>::platform;
    using address_type = Address;

public:
    bool isOpen() const { return m_resource_handler.open(); }

    void setNonBlocking(bool non_blocking){
        io::FileStatusFlags flags{m_resource_handler.getHandle()};
        if (non_blocking != flags.nonBlock()){
            flags.setNonBlock(non_blocking);
        }
    }

    bool setAddress(const Address & addr) { return setPathname(addr.getAddress()); }
    bool setPathname(std::string pathname) {
        if (isOpen()) return false;
        m_fifo_pathname = std::move(pathname); 
        return true;
    }

    bool nonBlocking() const {
        io::FileStatusFlags flags{m_resource_handler.getHandle()};
        return flags.nonBlock();
    }

    std::string pathname() const { return m_fifo_pathname; }
    bool close() { return m_resource_handler.close(); }

protected:
    NamedPipeDevice() {}
    NamedPipeDevice(std::string pathname) :
        m_fifo_pathname{std::move(pathname)}
    {}

protected:
    template<typename P>
    static bool openImpl(...){
        std::cout << "open ...\n";
        return false;
    }

    template<typename P, typename = typename P::io_profile>
    bool openImpl(const std::string & pathname, bool non_blocking = false){
        auto access_mode{ std::is_same_v<
                    typename P::io_profile,
                    read_only_profile> ? io::EAccessMode::E_READ_ONLY : io::EAccessMode::E_WRITE_ONLY };

        return openPipe(pathname, access_mode, non_blocking);
    }

    bool openPipe(const std::string & pathname, io::EAccessMode access_mode, bool non_blocking){
        using namespace utils::unx;
        std::cout << "NamedPipeDevice<T>::openPipe non blocking: " << std::boolalpha << non_blocking
                  << " path: " << pathname << "\n";

        std::cout << "NamedPipeDevice<T>::openPipe pathname: " << pathname << "; handler open: " << m_resource_handler.open() << "\n";
        if (pathname.empty() || m_resource_handler.open()){
            std::cerr << "NamedPipeDevice<T>::openPipe pathname empty or device already open\n";
            return false;
        }

        bool new_pipe_created{false};
        if (!LinuxIOUtilities::exists(pathname)){
            new_pipe_created = LinuxIOUtilities::makefifo(pathname);
            if (!new_pipe_created){
                std::cerr << "NamedPipeDevice<T>::openPipe() mkfifo failed\n";
                return false;
            }
        }

        int open_flags = io::getAccessModeFlag(access_mode);
        std::cout << "NamedPipeDevice<T>::openPipe open file flags " << open_flags <<"\n";

        open_flags |= O_CLOEXEC;
        if (non_blocking) open_flags |= O_NONBLOCK;

        if (handle_type fd{::open(pathname.c_str(), open_flags)}; -1 != fd){
            std::cout << "NamedPipeDevice<T>::openPipe open::\n";
            m_resource_handler.acquire(fd);
            if (pathname != m_fifo_pathname) m_fifo_pathname = pathname;
            if (new_pipe_created) ::unlink(pathname.c_str());
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
