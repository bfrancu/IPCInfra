#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <cstring>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include <string>
#include <algorithm>
#include <random>

#include "LinuxIOUtilities.h"
#include "FileStatusFlags.h"
#include "FilePermissions.h"
#include "FileInfo.h"

#include "utilities.hpp"
#include "sys_call_eval.h"

namespace infra
{
namespace utils
{
namespace unx
{

namespace
{
constexpr char END_LINE_CHAR{'\n'};
constexpr char PATH_SEPARATOR{'/'};
constexpr unsigned int PATH_MAX{4096};
constexpr int NULL_DESCRIPTOR{-1};
const std::string TEMP_FILE_EXTENSION{".tmp"};

constexpr std::array<char, 62> CHARSET{
        '0','1','2','3','4','5','6','7','8','9',
        'A','B','C','D','E','F','G','H','I','J','K',
        'L','M','N','O','P','Q','R','S','T','U',
        'V','W','X','Y','Z','a','b','c','d','e','f',
        'g','h','i','j','k', 'l','m','n','o','p',
        'q','r','s','t','u','v','w','x','y','z'
};

}

std::pair<std::string, std::string>
LinuxIOUtilities::getDirectoryAndFileName(const std::string &full_path)
{
    std::string file_path;
    std::string file_name;
    size_t prev_pos = 0;
    size_t next_pos = full_path.find_first_of(PATH_SEPARATOR);
    int offset = 0;
    while (std::string::npos != next_pos)
    {
        offset = 1;
        // to exclude the corner cases where both are 0 for example
        if (prev_pos != next_pos)
        {
            std::string sub = full_path.substr(prev_pos, next_pos - prev_pos);
            file_path.append(sub);
            prev_pos = next_pos;
        }
        next_pos = full_path.find_first_of(PATH_SEPARATOR, next_pos + 1);
    }

    file_name = full_path.substr(prev_pos + offset, full_path.length() - prev_pos);

    return std::make_pair(file_path, file_name);
}

std::string LinuxIOUtilities::fullPath(const std::string &filename, const std::string &directory)
{
    std::string ret{directory};
    if (!directory.empty() && !filename.empty())
    {
        ret.push_back(PATH_SEPARATOR);
        ret = ret.append(filename);
    }
    return ret;
}

int LinuxIOUtilities::openNewFile(const std::string &file_path, io::EAccessMode acces_mode, int flags, int mode)
{
    using namespace io;
    int ret_fd{NULL_DESCRIPTOR};
    int acces_mode_flag = getAccessModeFlag(acces_mode);

    // add the O_CREAT and O_EXCL flags just to be sure
    int new_file_flags = flags | acces_mode_flag | O_CREAT | O_EXCL | O_CLOEXEC;

    // this will be used when re-opening the file
    int open_file_flags = new_file_flags & ~O_CREAT;
    open_file_flags = open_file_flags & ~O_EXCL;

    if (0 == mode)
    {
        mode = static_cast<unsigned int>(FilePermissions{EFilePermission::E_USER_READ, EFilePermission::E_USER_WRITE});
    }

    ret_fd = ::open(file_path.c_str(), new_file_flags, mode);

    if (NULL_DESCRIPTOR == ret_fd)
    {
        // to check errno to see if the file already exists. if that's the case open without O_CREAT | O_EXCL flags
        // do not pass the mode parameter if the O_CREAT flag is omitted
        std::cerr << "File::openNewFile() " << file_path  << " opened failed with " << errno << "\n";
        if (utils::to_underlying(EFileIOError::E_ERROR_EEXIST) == errno)
        {
            std::cout << "File::openNewFile() reopening the file without the O_CREAT flag\n";
            ret_fd = ::open(file_path.c_str(), open_file_flags);
        }
    }
    if (NULL_DESCRIPTOR != ret_fd)
    {
        // check if the actual flags on the opened fd are the same as the requested ones
        // they usually are not, because of the umask
        FileInfo info{ret_fd};
        auto opened_mode_flags = info.permissions().getModeFlags();

        // if they are different apply fchmod to change them to the ones
        // initially requested
        if (static_cast<int>(opened_mode_flags) == mode)
        {
            return ret_fd;
        }

        if (-1 != fchmod(ret_fd, mode))
        {
            // reopen the file for the changes to take place
            // here we must make sure the O_CREAT | O_EXCL files are not applied
            ::close(ret_fd);
            ret_fd = ::open(file_path.c_str(), open_file_flags);
            if (NULL_DESCRIPTOR == ret_fd)
            {
                std::cerr << "File::openNewFile() " << file_path << " Failed with error " << errno << "\n";
            }
            else
            {
                FileInfo info{ret_fd};
                FileStatusFlags status_flags{ret_fd};
                std::cout << "File::openNewFile() opened file info: \n";
                std::cout << info;
                std::cout << "File::openNewFile() opened file status flags\n";
                std::cout << status_flags;
            }
        }
        else
        {
            std::cerr << "File::openNewFile() fchmod failed with " << errno << "\n";
        }
    }
    return ret_fd;
}

int LinuxIOUtilities::openExistingFile(const std::string &file_path, io::EAccessMode access_mode, int flags)
{
    return ::open(file_path.c_str(), getAccessModeFlag(access_mode) | flags | O_CLOEXEC);
}

bool LinuxIOUtilities::exists(const std::string &pathname)
{
    return (0 == ::access(pathname.c_str(), F_OK));
}

bool LinuxIOUtilities::existingDirectory(const std::string &directory)
{
    std::cout << "LinuxIOUtilities::existingDirectory ___" << directory << "___\n";
    io::FileInfo info{directory};
    return io::EFileType::E_DIRECTORY == info.type() && exists(directory);
}

bool LinuxIOUtilities::remove(const std::string &pathname)
{
    return (-1 != ::unlink(pathname.c_str()));
}

std::string LinuxIOUtilities::getCurrentDir()
{
    std::string ret;
    char buffer_arr[PATH_MAX];
    if (nullptr != ::getcwd(buffer_arr, PATH_MAX))
    {
       ret.reserve(::strnlen(buffer_arr, PATH_MAX));
       ret.append(buffer_arr);
    }
    return ret;
}

int64_t LinuxIOUtilities::availableSize(int fd){
     off_t current_pos = ::lseek(fd, 0, SEEK_CUR);
     if (-1 == current_pos) return -1;

     off_t end_pos = ::lseek(fd, 0, SEEK_END);
     ::lseek(fd, current_pos, SEEK_SET);

     return  end_pos - current_pos;
}

std::string LinuxIOUtilities::generateTempFileName(size_t length)
{
    std::string ret(length, '0');
    //1) create a non-deterministic random number generator
    std::default_random_engine rng(std::random_device{}());
    //2) create a random number "shaper" that will give
    //   us uniformly distributed indices into the character set
    std::uniform_int_distribution<> dist(0, CHARSET.size()-1);

    auto getRandomChar = [&dist, &rng]() ->char { return CHARSET[dist(rng)]; };
    std::generate(ret.begin(), ret.end(), getRandomChar);

    ret.append(TEMP_FILE_EXTENSION);
    return ret;
}

bool LinuxIOUtilities::copyTo(const std::string &original_name, std::string copy_name)
{
    bool ret{false};

    if (!exists(original_name))
    {
        std::cerr << original_name << " does not exist\n";
        return ret;
    }

    if(exists(copy_name))
    {
        std::cerr << copy_name << " already exists\n";
        return ret;
    }

    int existing_fd = openExistingFile(original_name, io::EAccessMode::E_READ_ONLY, 0);
    if (NULL_DESCRIPTOR == existing_fd)
    {
        std::cerr << "error opening original file: " << errno << "\n";
        return ret;
    }
    else
    {
        std::cout << "original file opened successfully\n";
    }

    io::FilePermissions existing_permissions{io::FileInfo(existing_fd).permissions()};
    io::FileStatusFlags existing_flags{io::FileStatusFlags(existing_fd)};


    int new_fd = openNewFile(copy_name, io::EAccessMode::E_WRITE_ONLY, static_cast<int>(existing_permissions.getModeFlags()), existing_flags.getFlags());
    if (NULL_DESCRIPTOR == new_fd)
    {
        std::cerr << "error opening new file: " << errno << "\n";
        return ret;
    }

    std::string line_buffer;
    while (0 < readLine(existing_fd, line_buffer))
    {
        write(new_fd, line_buffer);
        line_buffer.clear();
    }

    ret = !(0 > io::FileInfo(new_fd).availableSize());

    ::close(existing_fd);    
    ::close(new_fd);

    return ret;
}

bool LinuxIOUtilities::makefifo(const std::string &pathname)
{
    std::cout << "LinuxIOUtilities::makefifo\n";
    auto [directory, filename] = LinuxIOUtilities::getDirectoryAndFileName(pathname);

    std::cout << "LinuxIOUtilities::makefifo directory: " << directory
              << ", filename: " << filename << "\n";

    if (LinuxIOUtilities::existingDirectory(directory)){
        /* So we get the permissions we want */
        ::umask(0);
        return sys_call_noerr_eval(::mkfifo, pathname.c_str(), S_IRUSR | S_IWUSR | S_IWGRP);
    }

    return false;
}

size_t LinuxIOUtilities::read(int fd, std::string &result)
{
    //std::cout << "LinuxIOUtilities::read(int, std::string &)\n";
    char read_buffer[READ_BUFFER_SIZE];
    ssize_t total_size_read{0};
    int read_count_per_call{0};

    if (auto in_size = availableSize(fd); -1 != in_size){
        //std::cout << "LinuxIOUtilities::read() available size: " << in_size <<"\n";
        if (static_cast<int>(result.size()) < in_size){
            result.resize(in_size);
        }
        total_size_read = readInBuffer(fd, result.size() - 1, result.data());
        return total_size_read;
    }
    else
    {
        //std::cout << "LinuxIOUtilities::read() available size not available: " << in_size <<"\n";
    }

    memset(read_buffer, 0, READ_BUFFER_SIZE);
    //std::cout << "LinuxIOUtilities::read() before loop\n";
    for (;;){
        read_count_per_call = ::read(fd, read_buffer, READ_BUFFER_SIZE);
        if (-1 == read_count_per_call){
            if (EINTR == errno) continue;
            else{
                total_size_read = -1;
                break;
            }
        }

        result.append(read_buffer, read_count_per_call);
        total_size_read += static_cast<size_t>(read_count_per_call);

        // the buffer is not fully filled. no more data available
        if (READ_BUFFER_SIZE != read_count_per_call) break;
    }
    return total_size_read;
}

size_t LinuxIOUtilities::readLine(int fd, std::string &result)
{
    char read_buffer[READ_BUFFER_SIZE];
    const char * read_buffer_end{nullptr};
    size_t total_size_read{0};
    int read_count_per_call{0};

    result.reserve(READ_BUFFER_SIZE);
    memset(read_buffer, 0, READ_BUFFER_SIZE);

    for(;;){
        read_count_per_call = ::read(fd, read_buffer, READ_BUFFER_SIZE);
        if (-1 == read_count_per_call){
            if (EINTR == errno) continue;
            else break;
        }

        if(0 == read_count_per_call) break;

        read_buffer_end = read_buffer + read_count_per_call;
        auto endline_pos = std::find_if(const_cast<const char*>(read_buffer), read_buffer_end, [] (char ch){ return END_LINE_CHAR == ch; });

        auto copiable_chars_count = (endline_pos == read_buffer_end ? read_count_per_call
                : static_cast<size_t>(std::distance(std::cbegin(read_buffer), endline_pos)));

        result.append(read_buffer, copiable_chars_count);
        total_size_read += copiable_chars_count;

        if (static_cast<int>(copiable_chars_count) != read_count_per_call) break;
    }

    result.shrink_to_fit();
    return total_size_read;
}

size_t LinuxIOUtilities::readInBuffer(int fd, size_t buffer_len, char *out_buffer)
{
    //std::cout << "LinuxIOUtilities::readInBuffer() buffer len: " << buffer_len << "\n";
    ssize_t size_read{0};
    ssize_t read_count_per_call{0};
    memset(out_buffer, 0, buffer_len);

    if (0 == buffer_len)
    {
        return size_read;
    }

    for (;;){
        read_count_per_call = ::read(fd, out_buffer, buffer_len);
        if (-1 == read_count_per_call){
            if (EINTR == errno) continue;
            else{
                size_read = -1;
                break;
            }
        }

        size_read += read_count_per_call;
        //std::cout << "size read: " << size_read << "n";

        // the buffer is not fully filled. no more data available
        if (static_cast<ssize_t>(buffer_len) != read_count_per_call) break;
    }
    return size_read;
}

/*ssize_t LinuxIOUtilities::write(int fd, const std::string &data)
{
    auto data_len{data.length()};
    ssize_t total_size_written{0};
    ssize_t write_count_per_call{0};

    while (total_size_written < static_cast<ssize_t>(data_len))
    {
        write_count_per_call = ::write(fd, data.data() + total_size_written,
                                               data_len - total_size_written);
        if (-1 == write_count_per_call){
            if (EINTR == errno) continue;
            else break;
        }

        total_size_written += write_count_per_call;
    }

    return total_size_written;
}*/

ssize_t LinuxIOUtilities::write(int fd, std::string_view data)
{
    //std::cout << "IOPolicy::write(handle)\n";
    auto data_len{data.length()};
    ssize_t total_size_written{0};
    ssize_t write_count_per_call{0};

    while (total_size_written < static_cast<ssize_t>(data_len))
    {
        write_count_per_call = ::write(fd, data.data() + total_size_written,
                data_len - total_size_written);
        if (-1 == write_count_per_call){
            if (EINTR == errno) continue;
            else break;
        }

        total_size_written += write_count_per_call;
    }

    return total_size_written;
}

ssize_t LinuxIOUtilities::send(int fd, std::string_view data, size_t max_size, SocketIOFlags flags)
{
    return ::send(fd, data.data(), max_size, static_cast<int>(flags));
}

/*
template<typename RecvFunc, typename... Args>
ssize_t LinuxIOUtilities::recvInBuffer(RecvFunc && callable, int fd, size_t max_size, char *buffer, int flags, Args&&... additional_args)
{
    memset(buffer, 0, max_size);
    ssize_t read_count_per_call{0};
    ssize_t ret_total_size_read{0};
    //call blocks until length bytes have been received
    if (static_cast<int>(io::ESocketIOFlag::E_MSG_WAITALL) & flags)
    {
        return std::forward<RecvFunc>(callable)(fd, buffer, max_size, flags, std::forward<Args>(additional_args)...);
    }

    for (;;)
    {
        read_count_per_call = std::forward<RecvFunc>(callable)(fd, buffer, max_size, flags, std::forward<Args>(additional_args)...);

        if (-1 == read_count_per_call){
            if (EINTR == errno) continue;
            else break;
        }
        if (0 == read_count_per_call) break;

        ret_total_size_read += read_count_per_call;

        if (static_cast<ssize_t>(max_size) >= read_count_per_call) break;
    }
    return ret_total_size_read;
}

template<typename RecvFunc, typename... Args>
ssize_t LinuxIOUtilities::recv(RecvFunc && callable, int fd, size_t max_length, std::string & result, int flags, Args&&... additional_args)
{
    result.clear();
    char buf[READ_BUFFER_SIZE];
    ssize_t ret_total_size_read{0};
    ssize_t read_count_per_call{0};
    memset(buf, 0, READ_BUFFER_SIZE);

    //call blocks until length bytes have been received
    if (static_cast<int>(io::ESocketIOFlag::E_MSG_WAITALL) & flags)
    {
        ret_total_size_read = std::forward<RecvFunc>(callable)(fd, buf, max_length, flags, std::forward<Args>(additional_args)...);

        if (ret_total_size_read > 0){
            result.assign(buf, static_cast<size_t>(ret_total_size_read));
        }
    }

    for (;;)
    {
        read_count_per_call = std::forward<RecvFunc>(callable)(fd, buf, max_length, flags, std::forward<Args>(additional_args)...);

        if (-1 == read_count_per_call){
            if (EINTR == errno) continue;
            else break;
        }
        if (0 == read_count_per_call) break;

        if((ret_total_size_read + read_count_per_call) > static_cast<ssize_t>(max_length)){
            result.reserve(max_length - static_cast<size_t>(ret_total_size_read));
            result.append(buf, max_length - static_cast<size_t>(ret_total_size_read));
            ret_total_size_read = static_cast<ssize_t>(max_length);
            break;
        }

        if (auto capacity = result.capacity(); capacity + read_count_per_call < result.max_size()){
            result.reserve(capacity + read_count_per_call);
        }

        result.append(buf, static_cast<size_t>(read_count_per_call));
        ret_total_size_read += read_count_per_call;

        //the buffer is not fully filled. no more data available
        if (READ_BUFFER_SIZE != read_count_per_call) break;
    }

    return ret_total_size_read;
}
*/

} //lnx
} //utils
} //infra
