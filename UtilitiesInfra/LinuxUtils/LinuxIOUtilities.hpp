#ifndef LINUXIOUTILITIES_HPP
#define LINUXIOUTILITIES_HPP
#include <sys/socket.h>
#include <errno.h>
#include <cstring>
//#include <iostream>

namespace infra
{
namespace utils
{
namespace unx
{

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
    //std::cout << "LinuxIOUtilities::recv()\n";
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
            //std::cout << "LinuxIOUtilities::recv() 1: result: " << result << "\n";
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
        //std::cout << "LinuxIOUtilities::recv() 2: result: " << result << "\n";
        ret_total_size_read += read_count_per_call;

        //the buffer is not fully filled. no more data available
        if (READ_BUFFER_SIZE != read_count_per_call) break;
    }

    return ret_total_size_read;
}

}//unx
}//utils
}//infra
#endif
