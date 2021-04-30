#ifndef LINUXIOUTILITIES_H
#define LINUXIOUTILITIES_H
#include <stdint.h>
#include <unistd.h>
#include <iosfwd>
#include <utility>
#include <string_view>

#include "FileIODefinitions.h"
#include "SocketTypes.h"
#include "enum_flag.h"

namespace infra
{

using SocketIOFlags = enum_flag<io::ESocketIOFlag>;

namespace utils
{
namespace unx
{

class LinuxIOUtilities
{
private:
    static constexpr unsigned int READ_BUFFER_SIZE{4096};
public:
    static bool remove(const std::string & pathname);
    static bool exists(const std::string & pathname);
    static bool existingDirectory(const std::string & directory);
    static std::string fullPath(const std::string & filename, const std::string & directory);
    static std::string getCurrentDir();
    static std::pair<std::string, std::string> getDirectoryAndFileName(const std::string & full_path);
    static int openNewFile(const std::string & file_path, io::EAccessMode access_mode, int flags, int mode);
    static int openExistingFile(const std::string &file_path, io::EAccessMode access_mode, int flags);
    static int64_t availableSize(int fd);
    std::string generateTempFileName(size_t length);
    static bool copyTo(const std::string & original_name, std::string copy_name);
    static bool makefifo(const std::string & pathname);
    static size_t read(int fd, std::string & result);
    static size_t readLine(int fd, std::string & result);
    static size_t readInBuffer(int fd, size_t buffer_len, char *buffer);
    //static ssize_t write(int fd, const std::string & data);
    static ssize_t write(int fd, std::string_view data);
    static ssize_t send(int fd, std::string_view data, size_t max_size,
                        SocketIOFlags flags = io::ESocketIOFlag::E_MSG_NO_FLAG);

    template<typename RecvFunc, typename... Args>
    static ssize_t recvInBuffer(RecvFunc && callable, int fd, size_t buffer_len, char *buffer, int flags, Args&&... additional_args);

    template<typename RecvFunc, typename... Args>
    static ssize_t recv(RecvFunc && callable, int fd, size_t max_length, std::string & result, int flags, Args&&... additional_args);
};

} //unx
} //utils
} // infra
#endif // LINUXIOUTILITIES_H

#include "LinuxIOUtilities.hpp"
