#include <fcntl.h>

#include <sys/stat.h>

#include "NamedPipeFactory.h"


#include "sys_call_eval.h"

namespace infra
{
/*
template<template<typename... > typename... Policies>
decltype(auto) NamedPipeFactory::getReadingEndpoint(std::string pathname, bool non_blocking)
{
    Host<WritingNamedPipeDevice<UnixResourceHandler>, Policies...> rd_pipe_device{};
    openPipeEndpoint(rd_pipe_device, pathname, non_blocking);
    return rd_pipe_device;
}
*/


/*template<template<typename... > typename... Policies>
decltype(auto) NamedPipeFactory::getWritingEndpoint(std::string pathname, bool non_blocking)
{
    // process specific not fifo specific
    // the handler will work for all SIGPIPE siignals emitted for the calling process
    std::call_once(sigpipe_ignored_flag, [] {signal(SIGPIPE, SIG_IGN);});

    Host<WritingNamedPipeDevice<UnixResourceHandler>, Policies...> wr_pipe_device{};
    openPipeEndpoint(wr_pipe_device, pathname, non_blocking);
    return wr_pipe_device;
}*/

/*template<typename NamedPipe>
void NamedPipeFactory::openPipeEndpoint(NamedPipe &device, std::string pathname, bool non_blocking)
{
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
}*/

bool NamedPipeFactory::makefifo(const std::string &pathname)
{
    std::cout << "NamedPipeFactory::makefifo\n";
    using namespace utils::unx;
    auto [directory, filename] = LinuxIOUtilities::getDirectoryAndFileName(pathname);

    std::cout << "NamedPipeFactory::makefifo directory: " << directory
              << ", filename: " << filename << "\n";

    if (LinuxIOUtilities::existingDirectory(directory)){
        /* So we get the permissions we want */
        ::umask(0);
        return sys_call_noerr_eval(::mkfifo, pathname.c_str(), S_IRUSR | S_IWUSR | S_IWGRP);
    }

    return false;
}

} // infra
