#ifndef DATAGRAMIOPOLICY_HPP
#define DATAGRAMIOPOLICY_HPP
#include <sys/socket.h>

#include <string_view>
#include <string>
#include <memory>

#include <iostream>

#include "SocketTypes.h"
#include "Policies/StreamIOPolicy.hpp"

namespace infra
{


template<typename Host, typename SocketDevice, typename = void>
class DatagramIOPolicy
{};

template<typename Host, typename SocketDevice>
class DatagramIOPolicy<Host,
                       SocketDevice,
                       std::enable_if_t<std::conjunction<IsUnixSocketDeviceT<SocketDevice>, IsDatagramSocketDeviceT<SocketDevice>>::value>>
        //: public crtp_base<DatagramIOPolicy<Host, SocketDevice>, Host>
        : public StreamIOPolicy<Host, SocketDevice>

{
    using handle_type         = typename device_traits<SocketDevice>::handle_type;
    using socket_address_type = typename socket_traits<SocketDevice>::socket_address_type;

public:
    ssize_t sendTo(const socket_address_type & peer_socket, std::string_view data, SocketIOFlags flags = SocketIOFlags{io::ESocketIOFlag::E_MSG_NO_FLAG}){
        return sendTo(peer_socket, data, data.length(), flags);
    }

    ssize_t sendTo(const socket_address_type & peer_socket, std::string_view data, size_t max_size, SocketIOFlags flags = SocketIOFlags{io::ESocketIOFlag::E_MSG_NO_FLAG}){
        static std::unique_ptr<sockaddr> p_dst_addr{std::make_unique<sockaddr>()};
        memset(p_dst_addr->sa_data, 0, sizeof(p_dst_addr->sa_data));
        peer_socket.getAddress(*p_dst_addr);
        return ::sendto(SocketDeviceAccess::getHandle(this->asDerived()),
                        data.data(),
                        max_size,
                        static_cast<int>(flags),
                        p_dst_addr.get(),
                        sizeof(sockaddr));
    }

    ssize_t recvFrom(size_t max_length, SocketIOFlags flags, std::string & out_data, socket_address_type & out_source){
        /*static char buf[this->READ_BUFFER_SIZE];

        static std::unique_ptr<sockaddr> p_from_addr{std::make_unique<sockaddr>()};
        static socklen_t addr_len = sizeof(sockaddr);
        memset(p_from_addr->sa_data, 0, sizeof(p_from_addr->sa_data));

        std::fill_n(buf, this->READ_BUFFER_SIZE, 0);

        ssize_t ret_total_size_read{ 0 };
        ssize_t read_count_per_call{ 0 };

        //call blocks until length bytes have been received
        if (SocketIOFlags(io::ESocketIOFlag::E_MSG_WAITALL) & flags){
            ret_total_size_read = ::recvfrom(SocketDeviceAccess::getHandle(this->asDerived()),
                                             buf,
                                             max_length,
                                             static_cast<int>(flags),
                                             p_from_addr.get(),
                                             &addr_len);
            if (ret_total_size_read > 0){
                out_data.reserve(ret_total_size_read);
                out_data.append(buf, static_cast<size_t>(ret_total_size_read));
            }
        }

        else{
            for (;;){
                read_count_per_call = ::recvfrom(SocketDeviceAccess::getHandle(this->asDerived()),
                                                 buf,
                                                 max_length,
                                                 static_cast<int>(flags),
                                                 p_from_addr.get(),
                                                 &addr_len);

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
                if (this->READ_BUFFER_SIZE != read_count_per_call) break;
            }
            out_source.setAddress(*p_from_addr);
        }
        return ret_total_size_read;*/

        using namespace std::placeholders;
        auto recvfrom_cb = std::bind(&DatagramIOPolicy::recvFromSysCall, this,
                                     SocketDeviceAccess::getHandle(this->asDerived()), _1, _2, _3, out_source);
        return this->recvLogic(recvfrom_cb, max_length, flags, out_data);
    }

    std::string recvFrom(size_t max_length, SocketIOFlags flags, socket_address_type & out_source){
         std::string ret;
         recvFrom(max_length, flags, &ret, out_source);
         return ret;
    };

private:
    ssize_t recvFromSysCall(int fd, char *p_buffer, size_t length, int flags, socket_address_type & recv_from_addr){
           std::cout << "recvfromSysCall\n";
           static std::unique_ptr<sockaddr> p_from_addr{std::make_unique<sockaddr>()};
           memset(p_from_addr->sa_data, 0, sizeof(p_from_addr->sa_data));
           socklen_t addr_len = sizeof(sockaddr);

           auto ret = ::recvfrom(fd, p_buffer, length, flags, p_from_addr.get(), &addr_len);
           if (-1 != ret) recv_from_addr.setAddress(*p_from_addr);
           return ret;
    }

};

} //infra
#endif // DATAGRAMIOPOLICY_HPP
