#ifndef DATAGRAMIOPOLICY_HPP
#define DATAGRAMIOPOLICY_HPP
#include <sys/socket.h>

#include <string_view>
#include <string>
#include <memory>

#include <iostream>

#include "LinuxUtils/LinuxIOUtilities.h"
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
                       std::enable_if_t<std::conjunction<IsUnixPlatformSocketDeviceT<SocketDevice>, IsDatagramSocketDeviceT<SocketDevice>>::value>>
        : public crtp_base<DatagramIOPolicy<Host, SocketDevice>, Host>
        //: public StreamIOPolicy<Host, SocketDevice>

{
    using handle_type         = typename device_traits<SocketDevice>::handle_type;
    using address_type = typename socket_traits<SocketDevice>::address_type;

public:
    ssize_t sendTo(const address_type & peer_socket, std::string_view data, SocketIOFlags flags = SocketIOFlags{io::ESocketIOFlag::E_MSG_NO_FLAG}){
        return sendTo(peer_socket, data, data.length(), flags);
    }

    ssize_t sendTo(const address_type & peer_socket, std::string_view data, size_t max_size, SocketIOFlags flags = SocketIOFlags{io::ESocketIOFlag::E_MSG_NO_FLAG}){
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

    ssize_t recvFromInBuffer(size_t buffer_len, SocketIOFlags flags, char *buffer, address_type & out_source){
        return utils::unx::LinuxIOUtilities::recvInBuffer(RECV_CB,
                                                          SocketDeviceAccess::getHandle(this->asDerived()),
                                                          buffer_len,
                                                          buffer,
                                                          static_cast<int>(flags),
                                                          out_source);
    }

    ssize_t recvFrom(size_t max_length, SocketIOFlags flags, std::string & out_data, address_type & out_source){
        return utils::unx::LinuxIOUtilities::recv(RECV_CB,
                                                  SocketDeviceAccess::getHandle(this->asDerived()),
                                                  max_length, 
                                                  out_data,
                                                  static_cast<int>(flags),
                                                  out_source);

    }

    std::string recvFrom(size_t max_length, SocketIOFlags flags, address_type & out_source){
        std::string ret;
        recvFrom(max_length, flags, &ret, out_source);
        return ret;
    };

private:
    static constexpr auto RECV_CB{[] (int fd, char *buffer, size_t length, int flags, address_type & recv_from_addr){
              std::cout << "DatagramIOPolicy::recvFromSysCall\n";
              static std::unique_ptr<sockaddr> p_from_addr{std::make_unique<sockaddr>()};
              memset(p_from_addr->sa_data, 0, sizeof(p_from_addr->sa_data));
              socklen_t addr_len = sizeof(sockaddr);

              auto ret = ::recvfrom(fd, buffer, length, flags, p_from_addr.get(), &addr_len);
              if (-1 != ret) recv_from_addr.setAddress(*p_from_addr);
              return ret;
         }
    };

};

} //infra
#endif // DATAGRAMIOPOLICY_HPP
