#ifndef DEVICE_ADDRESS_FACTORY_H
#define DEVICE_ADDRESS_FACTORY_H
#include "DeviceDefinitions.h"
#include "../Configuration/ConfigurationBook.h"
#include "Devices/Sockets/UnixSocketAddress.h"
#include "Devices/Pipes/NamedPipeAddress.h"

namespace infra
{
    //inline const char * const CONNECTION_SECTION{"CONNECTION_DETAILS"};

    class InetSocketAddressFactoryBase
    {
   protected:
       static inline const char * const SERVICE_PARAM{"SERVICE"};
       static inline const char * const HOST_PARAM{"HOST"};
       static inline const char * const LOCALHOST_VALUE{"LOCALHOST"};
       static inline const char * const LOCALHOST_VALUE_LC{"localhost"};

   protected:
       template<typename NetworkAddress>
       static inet::InetSocketAddress<NetworkAddress>
       createAddress(const config::ConfigurationBook & book, std::string_view section){
        std::string host_str_value;
        NetworkAddress network_addr;
        inet::InetSocketAddress<NetworkAddress> ret_addr;

        if(book.valueFor(config::ConfigurationAddress{section, HOST_PARAM}, host_str_value) &&
                book.valueFor(config::ConfigurationAddress{section, SERVICE_PARAM}, network_addr.port_number)){
            if(host_str_value == LOCALHOST_VALUE || host_str_value == LOCALHOST_VALUE_LC){
                network_addr.host_address.setAddressAny();
            }
            else {
                network_addr.host_address.setAddress(host_str_value);
            }
            ret_addr.setAddress(network_addr);
        }
        return ret_addr;
       }
    };

    template<std::size_t tag>
    constexpr bool isIPV4SocketDevice() {
        return tag == static_cast<std::size_t>(EDeviceType::E_IPV4_TCP_SOCKET_DEVICE) ||
               tag == static_cast<std::size_t>(EDeviceType::E_IPV4_UDP_SOCKET_DEVICE);
    }

    template<std::size_t tag>
    constexpr bool isIPV6SocketDevice() {
        return tag == static_cast<std::size_t>(EDeviceType::E_IPV6_TCP_SOCKET_DEVICE) ||
               tag == static_cast<std::size_t>(EDeviceType::E_IPV6_UDP_SOCKET_DEVICE);
    }

    template<std::size_t tag>
    constexpr bool isUnixSocketDevice(){
        return tag == static_cast<std::size_t>(EDeviceType::E_UNIX_STREAM_SOCKET_DEVICE) ||
               tag == static_cast<std::size_t>(EDeviceType::E_UNIX_DGRAM_SOCKET_DEVICE);
    }

    template<std::size_t tag>
    constexpr bool isNamedPipeDevice(){
        return tag == static_cast<std::size_t>(EDeviceType::E_READING_FIFO_DEVICE) ||
               tag == static_cast<std::size_t>(EDeviceType::E_WRITING_FIFO_DEVICE);
    }

    template<std::size_t tag, typename Enable = void>
    class DeviceAddressFactory
    {};
    
    /*
    template<std::size_t first, std::size_t second>
    constexpr bool equals() { return first == second; } 
    */


    template<std::size_t tag>
    class DeviceAddressFactory<tag, std::enable_if_t<isIPV4SocketDevice<tag>()>> : protected InetSocketAddressFactoryBase
    {
        using Base = InetSocketAddressFactoryBase;
   public:
        using DeviceAddressT = IPV4InetSocketAddress;
   public:
        static DeviceAddressT createAddress(const config::ConfigurationBook & book, std::string_view section){
            return Base::createAddress<IPV4NetworkAddress>(book, section);
        }
    };

    template<std::size_t tag>
    class DeviceAddressFactory<tag, std::enable_if_t<isIPV6SocketDevice<tag>()>> : protected InetSocketAddressFactoryBase
    {
        using Base = InetSocketAddressFactoryBase;
   public:
        using DeviceAddressT = IPV6InetSocketAddress;
   public:
        static DeviceAddressT createAddress(const config::ConfigurationBook & book,
                                          std::string_view section){
            return Base::createAddress<IPV6NetworkAddress>(book, section);
        }
    };

    template<std::size_t tag>
    class DeviceAddressFactory<tag, std::enable_if_t<isUnixSocketDevice<tag>()>>
    {
       static inline const char * const PATHNAME_PARAM{"PATHNAME"};
   public:
       using DeviceAddressT = unx::UnixSocketAddress;
   public:
       static DeviceAddressT createAddress(const config::ConfigurationBook & book, std::string_view section){
          DeviceAddressT ret_addr;
          if(unx::UnixAddress unx_addr;
              book.valueFor(config::ConfigurationAddress{section, PATHNAME_PARAM}, unx_addr.pathname)){
                ret_addr.setAddress(unx_addr);
            } 
            return ret_addr;
       }
    };

    template<std::size_t tag>
    class DeviceAddressFactory<tag, std::enable_if_t<isNamedPipeDevice<tag>()>>
    {
       static inline const char * const PATHNAME_PARAM{"PATHNAME"};
    public:
       using DeviceAddressT = NamedPipeAddress;
    public:
       static DeviceAddressT createAddress(const config::ConfigurationBook & book, std::string_view section){
           DeviceAddressT ret_addr;
           if(std::string pathname;
               book.valueFor(config::ConfigurationAddress{section, PATHNAME_PARAM}, pathname)){
                 ret_addr.setAddress(pathname);
              }
           return ret_addr;
       }
    };
} //infra

#endif
