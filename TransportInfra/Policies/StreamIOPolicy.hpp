#ifndef STREAMIOPOLICY_HPP
#define STREAMIOPOLICY_HPP
#include <sys/socket.h>

#include <string_view>
#include <string>
#include <memory>
#include <iostream>

#include "sys_call_eval.h"
#include "enum_flag.h"
#include "crtp_base.hpp"
#include "SocketTypes.h"
#include "Traits/socket_traits.hpp"
#include "Devices/Sockets/SocketDeviceAccess.hpp"
#include "LinuxUtils/LinuxIOUtilities.h"


namespace infra
{

using SocketIOFlags = enum_flag<io::ESocketIOFlag>;


template<typename Host, typename SocketDevice, typename = void>
class StreamIOPolicy
{};

template<typename Host, typename SocketDevice>
class StreamIOPolicy<Host,
                       SocketDevice,
                       std::enable_if_t<IsUnixPlatformSocketDeviceT<SocketDevice>::value>>
        : public crtp_base<StreamIOPolicy<Host, SocketDevice>, Host>
{
    using handle_type  = typename device_traits<SocketDevice>::handle_type;

public:
    ssize_t send(std::string_view data, SocketIOFlags flags = SocketIOFlags{io::ESocketIOFlag::E_MSG_NOSIGNAL}){
        return send(data, data.length(), flags);
    }

    ssize_t send(std::string_view data, size_t max_size, SocketIOFlags flags = SocketIOFlags{io::ESocketIOFlag::E_MSG_NOSIGNAL}){
        //if (io::ESocketState::E_STATE_CONNECTED != this->asDerived().getState()) return -1;
        return utils::unx::LinuxIOUtilities::send(SocketDeviceAccess::getHandle(this->asDerived()), data, max_size, flags);
    }

    ssize_t recvInBuffer(size_t buffer_len, SocketIOFlags flags, char *buffer){
        //if (io::ESocketState::E_STATE_CONNECTED != this->asDerived().getState()) return -1;
        return utils::unx::LinuxIOUtilities::recvInBuffer(RECV_CB,
                                                          SocketDeviceAccess::getHandle(this->asDerived()),
                                                          buffer_len,
                                                          buffer,
                                                          static_cast<int>(flags));
    }

    ssize_t recv(size_t max_length, SocketIOFlags flags, std::string & out_data){
        //if (io::ESocketState::E_STATE_CONNECTED != this->asDerived().getState()) return -1;
        //using namespace std::placeholders;
        //static auto recv_cb = std::bind(::recv, SocketDeviceAccess::getHandle(this->asDerived()), _1, _2, _3);
        //
        /*
        static auto recv_callable = [] (int sockfd, void *buffer, size_t length, int flags){
            return ::recv(sockfd, buffer, length, flags);
        };*/

        return utils::unx::LinuxIOUtilities::recv(RECV_CB,
                                                  SocketDeviceAccess::getHandle(this->asDerived()),
                                                  max_length,
                                                  out_data,
                                                  static_cast<int>(flags));
        //return recvLogic(recv_cb, max_length, flags, out_data);
    }

    std::string recv(size_t max_length, SocketIOFlags flags){
         std::string ret;
         recv(max_length, flags, &ret);
         return ret;
    };

protected:
    using recv_callable = std::function<ssize_t(char *, size_t, int)>;
    static constexpr auto RECV_CB{[] (int sockfd, void *buffer, size_t length, int flags){
                                      return ::recv(sockfd, buffer, length, flags);}};

    inline static constexpr unsigned int READ_BUFFER_SIZE{4096};

protected:
    ssize_t recvLogic(recv_callable sys_callable, size_t max_length, SocketIOFlags flags, std::string & out_data)
    {
        std::cout << "recvLogic\n";
        static char buf[READ_BUFFER_SIZE];
        std::fill_n(buf, READ_BUFFER_SIZE, 0);
        ssize_t ret_total_size_read{ 0 };
        ssize_t read_count_per_call{ 0 };

        //call blocks until length bytes have been received
        if (SocketIOFlags(io::ESocketIOFlag::E_MSG_WAITALL) & flags){
            ret_total_size_read = sys_callable(buf, max_length, static_cast<int>(flags));

            if (ret_total_size_read > 0){
                out_data.reserve(ret_total_size_read);
                out_data.append(buf, static_cast<size_t>(ret_total_size_read));
            }
        }

        else{
            for (;;){
                std::cout << "before callable\n";
                read_count_per_call = sys_callable(buf, READ_BUFFER_SIZE, static_cast<int>(flags));
                std::cout << "read count per call " << read_count_per_call << "\n";
                if (-1 == read_count_per_call){
                    if (EINTR == errno) continue;
                    else break;
                }
                if (0 == read_count_per_call) break;

                if((ret_total_size_read + read_count_per_call) > static_cast<ssize_t>(max_length)){
                    out_data.reserve(max_length - static_cast<size_t>(ret_total_size_read));
                    out_data.append(buf, max_length - static_cast<size_t>(ret_total_size_read));
                    ret_total_size_read = static_cast<ssize_t>(max_length);
                    break;
                }
                out_data.reserve(read_count_per_call);
                out_data.append(buf, static_cast<size_t>(read_count_per_call));
                ret_total_size_read += read_count_per_call;

                //the buffer is not fully filled. no more data available
                if (READ_BUFFER_SIZE != read_count_per_call) break;
            }
        }
        return ret_total_size_read;
    }
};

} //infra
#endif // STREAMIOPOLICY_HPP
