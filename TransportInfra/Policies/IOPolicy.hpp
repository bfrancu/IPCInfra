#ifndef IOPOLICY_HPP
#define IOPOLICY_HPP

#include <stdint.h>
#include <unistd.h>
#include <cstring>

#include <string>
#include <algorithm>
#include <type_traits>

#include "crtp_base.hpp"
#include "Devices/GenericDeviceAccess.hpp"
#include "Traits/device_traits.hpp"
#include "Traits/device_constraints.hpp"
#include "LinuxUtils/LinuxIOUtilities.h"

namespace infra
{

template<typename Host, typename Device, typename = std::void_t<>>
class GenericIOPolicy{};

template<typename Host, typename Device>
class GenericIOPolicy<Host, Device, std::void_t<traits::UnixDevice<Device>>>
        : public crtp_base<GenericIOPolicy<Host, Device>, Host>
{
     using handle_type = typename device_traits<Device>::handle_type;
     using base = crtp_base<GenericIOPolicy<Host, Device>, Host>;

public:
    ssize_t read(std::string & result){ return readImpl(GenericDeviceAccess::getHandle(this->asDerived()), result);}

    size_t readLine(std::string & result){ return readLine(GenericDeviceAccess::getHandle(this->asDerived()), result); }

    ssize_t readInBuffer(size_t buffer_len, char *buffer){ return readInBuffer2(GenericDeviceAccess::getHandle(this->asDerived()), buffer_len, buffer); }

    ssize_t write(const std::string & data) { return write(GenericDeviceAccess::getHandle(this->asDerived()), data); }

protected:
    static size_t readImpl(handle_type handle, std::string & result){
        char read_buffer[READ_BUFFER_SIZE];
        ssize_t total_size_read{0};
        int read_count_per_call{0};

        if (auto in_size = utils::unx::LinuxIOUtilities::availableSize(handle); -1 != in_size){
            if (static_cast<int>(result.size()) < in_size){
                result.resize(in_size);
            }
            total_size_read = readInBuffer2(handle, result.size() - 1, result.data());
            return total_size_read;
        }

        memset(read_buffer, 0, READ_BUFFER_SIZE);
        for (;;){
            read_count_per_call = ::read(handle, read_buffer, READ_BUFFER_SIZE);
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

    static ssize_t readInBuffer2(handle_type handle, std::size_t buffer_len, char *out_buffer)
    {
        ssize_t size_read{0};
        ssize_t read_count_per_call{0};
        memset(out_buffer, 0, buffer_len);

        for (;;){
            read_count_per_call = ::read(handle, out_buffer, buffer_len);
            if (-1 == read_count_per_call){
                if (EINTR == errno) continue;
                else{
                    size_read = -1;
                    break;
                }

                size_read += read_count_per_call;

                // the buffer is not fully filled. no more data available
                if (static_cast<ssize_t>(buffer_len) != read_count_per_call) break;
            }
        }
        return size_read;
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

    static ssize_t readInBuffer(handle_type handle, size_t buffer_len, char *buffer){
        ssize_t size_read{0};
        for (;;)
        {
            ssize_t read_count = ::read(handle, buffer, buffer_len);
            if (-1 == read_count){
                if(EINTR == errno) continue;
                else{
                    size_read = -1;
                    break;
                }
            }

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
