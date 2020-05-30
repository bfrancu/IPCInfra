#ifndef NAMEDPIPEFACTORY_H
#define NAMEDPIPEFACTORY_H
#include <signal.h>

#include <mutex>

#include "NamedPipeDevice.hpp"
#include "Host.hpp"
#include "LinuxUtils/LinuxIOUtilities.h"


namespace infra
{

//template<typename, typename>
//class NamedPipeDevice;

class NamedPipeFactory
{
public:

    template<template<typename... > typename... Policies>
    inline decltype(auto) getReadingEndpoint(std::string pathname, bool non_blocking = true){
        Host<ReadingNamedPipeDevice<UnixResourceHandler>, Policies...> rd_pipe_device{};
        openPipeEndpoint(rd_pipe_device, pathname, non_blocking);
        return rd_pipe_device;
    }

    template<template<typename... > typename... Policies>
    inline decltype(auto) getWritingEndpoint(std::string pathname, bool non_blocking = true){
        // process specific not fifo specific
        // the handler will work for all SIGPIPE siignals emitted for the calling process
        std::call_once(sigpipe_ignored_flag, [] {signal(SIGPIPE, SIG_IGN);});

        Host<WritingNamedPipeDevice<UnixResourceHandler>, Policies...> wr_pipe_device{};
        openPipeEndpoint(wr_pipe_device, pathname, non_blocking);
        return wr_pipe_device;
    }

private:
    template<typename NamedPipe>
    inline void openPipeEndpoint(NamedPipe & device, std::string pathname, bool non_blocking){
        std::cout << "NamedPipeFactory::openPipeEndpoint\n";
        using namespace utils::unx;
        bool new_pipe_created{false};

        if (!LinuxIOUtilities::exists(pathname)){
            new_pipe_created = makefifo(pathname);
            if (!new_pipe_created) return;
        }

        device.open(pathname, non_blocking);
        if(new_pipe_created && device.isOpen()){
            // the fifo will be deleted only if this is the creating process
            // and only after all file descriptions are closed
            ::unlink(pathname.c_str());
        }
    }

    bool makefifo(const std::string & pathname);

private:
    static inline std::once_flag sigpipe_ignored_flag{};
};

} // infra
#endif // NAMEDPIPEFACTORY_H
