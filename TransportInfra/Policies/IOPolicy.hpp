#ifndef IOPOLICY_HPP
#define IOPOLICY_HPP

#include <stdint.h>
#include <unistd.h>
#include <cstring>

#include <string>
#include <algorithm>
#include <type_traits>

#include "crtp_base.hpp"
#include "Traits/device_traits.hpp"
#include "LinuxUtils/LinuxIOUtilities.h"

namespace infra
{

template<typename Host, typename Device, typename = std::void_t<>>
class GenericIOPolicy{};

/*
template <typename Host, typename Device>
using GenericIOPolicyT = GenericIOPolicy<Host, Device, std::void_t<std::enable_if_t<HasUnixHandleTypeT<Device>::value>>>;
*/

//template <typename Host, typename Device>
//using GenericIOPolicyT = GenericIOPolicy<Host, Device, std::void_t<std::enable_if_t<std::is_same_v<int,double>>>>;

template<typename Host, typename Device>
class GenericIOPolicy<Host, Device, std::void_t<std::enable_if_t<HasUnixHandleTypeT<Device>::value>>>
        : public crtp_base<GenericIOPolicy<Host, Device, std::void_t<std::enable_if_t<HasUnixHandleTypeT<Device>::value>>>,
                                           Host>
{
     using handle_type = typename device_traits<Device>::handle_type;

public:
    size_t read(std::string & result){ return read(this->asDerived().getHandle(), result);}

    size_t readLine(std::string & result){ return readLine(this->asDerived().getHandle(), result); }

    size_t readInBuffer(size_t buffer_len, char *buffer){ return readInBuffer(this->asDerived().getHandle(), buffer_len, buffer); }

    ssize_t write(const std::string & data) { return write(this->asDerived().getHandle(), data); }

protected:
    static size_t read(handle_type handle, std::string & result){
        char read_buffer[READ_BUFFER_SIZE];
        size_t total_size_read{0};
        int read_count_per_call{0};

        if (auto in_size = utils::unx::LinuxIOUtilities::availableSize(handle);
            -1 != in_size){
            result.reserve(in_size);
        }

        memset(read_buffer, 0, READ_BUFFER_SIZE);
        for (;;){
            read_count_per_call = ::read(handle, read_buffer, READ_BUFFER_SIZE);
            if (-1 == read_count_per_call){
                if (EINTR == errno) continue;
                else break;
            }

            result.append(read_buffer, read_count_per_call);
            total_size_read += static_cast<size_t>(read_count_per_call);

            // the buffer is not fully filled. no more data available
            if (READ_BUFFER_SIZE != read_count_per_call) break;
        }
        return  total_size_read;
    }

    static size_t readLine(handle_type handle, std::string & result){
        char read_buffer[READ_BUFFER_SIZE];
        const char * read_buffer_end{nullptr};
        size_t total_size_read{0};
        int read_count_per_call{0};

        result.reserve(READ_BUFFER_SIZE);
        memset(read_buffer, 0, READ_BUFFER_SIZE);

        for(;;){
            read_count_per_call = ::read(handle, read_buffer, READ_BUFFER_SIZE);
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

    static size_t readInBuffer(handle_type handle, size_t buffer_len, char *buffer){
        size_t size_read{0};
        for (;;)
        {
            ssize_t read_count = ::read(handle, buffer, buffer_len);
            if (-1 == read_count && EINTR == errno) continue;

            if (read_count > 0)
            {
                size_read = static_cast<size_t>(read_count);
            }
            break;
        }
        return size_read;
    }

    static ssize_t write(handle_type handle, const std::string & data){
        auto data_len{data.length()};
        ssize_t total_size_written{0};
        ssize_t write_count_per_call{0};

        while (total_size_written < static_cast<ssize_t>(data_len))
        {
            write_count_per_call = ::write(handle, data.data() + total_size_written,
                                                   data_len - total_size_written);
            if (-1 == write_count_per_call){
                if (EINTR == errno) continue;
                else break;
            }

            total_size_written += write_count_per_call;
        }

        return total_size_written;
    }

private:
    static inline constexpr unsigned int READ_BUFFER_SIZE{4096};
    static inline constexpr char END_LINE_CHAR{'\n'};
};

} //infra

#endif // IOPOLICY_HPP
