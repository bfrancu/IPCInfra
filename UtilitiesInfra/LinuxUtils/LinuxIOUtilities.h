#ifndef LINUXIOUTILITIES_H
#define LINUXIOUTILITIES_H
#include <stdint.h>
#include <unistd.h>
#include <iosfwd>
#include <utility>

#include "FileIODefinitions.h"

namespace infra
{
namespace utils
{
namespace unx
{

class LinuxIOUtilities
{
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
    static ssize_t write(int fd, const std::string & data);
};

} //unx
} //utils
} // infra
#endif // LINUXIOUTILITIES_H
