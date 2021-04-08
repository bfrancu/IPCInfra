#include <algorithm>
#include <cctype>

#include "ConnectorClient.h"
#include "ConfigurationBook.h"
#include "Devices/DeviceDefinitions.h"

//#include "../Configuration/ConfigurationBook.h"

namespace
{
const char *const DEV_TYPE_KEY{"DEVICE_TYPE"};
const char *const SOCKET_TYPE_KEY{"SOCKET_TYPE"};
const char *const SOCKET_DOMAIN_KEY{"SOCKET_DOMAIN"};
const char *const PROFILE_KEY{"PROFILE"};
/*
 * TODO Don't forget to convert all chars to lower case before comparing
 */
const char *const DEV_TYPE_VAL_SOCKET{"socket"};
const char *const DEV_TYPE_VAL_PIPE{"pipe"};
const char *const SOCKET_DOMAIN_VAL_UNIX{"unix"};
const char *const SOCKET_DOMAIN_VAL_IPV4{"ipv4"};
const char *const SOCKET_DOMAIN_VAL_IPV6{"ipv6"};
const char *const SOCKET_TYPE_VAL_STREAM{"stream"};
const char *const SOCKET_TYPE_VAL_DGRAM{"datagram"};
const char *const READ_PROFILE_VAL{"read"};
const char *const WRITE_PROFILE_VAL{"write"};

}

namespace infra
{

void toLower(std::string & value){
    std::for_each(value.begin(), value.end(), 
                  [] (char & ch) { ch = std::tolower(ch); });
}


DeviceTypeReader::SockTypeMap DeviceTypeReader::SocketConfigInfoToType{
    {SOCKET_TYPE_VAL_DGRAM, { {SOCKET_DOMAIN_VAL_IPV4,
                               infra::EDeviceType::E_IPV4_UDP_SOCKET_DEVICE},
                              {SOCKET_DOMAIN_VAL_IPV6,
                               infra::EDeviceType::E_IPV4_UDP_SOCKET_DEVICE},
                              {SOCKET_DOMAIN_VAL_UNIX, 
                               infra::EDeviceType::E_UNIX_DGRAM_SOCKET_DEVICE}
                            }},
   {SOCKET_TYPE_VAL_STREAM, { {SOCKET_DOMAIN_VAL_IPV4,
                               infra::EDeviceType::E_IPV4_TCP_SOCKET_DEVICE},
                              {SOCKET_DOMAIN_VAL_IPV6,
                               infra::EDeviceType::E_IPV4_TCP_SOCKET_DEVICE},
                              {SOCKET_DOMAIN_VAL_UNIX, 
                               infra::EDeviceType::E_UNIX_STREAM_SOCKET_DEVICE}
                             }}
};

int DeviceTypeReader::getDeviceType(const infra::config::ConfigurationBook & book,
                                                  std::string_view section)
{
    std::cout << "DeviceTypeReader::getDeviceType()\n";
    using namespace infra;
    using namespace infra::config;
    EDeviceType ret_dev{EDeviceType::E_UNDEFINED_DEVICE};

    std::string dev_type;
    if (!(book.valueFor(ConfigurationAddress{section, DEV_TYPE_KEY}, dev_type))) {
        return ret_dev;
    }

    toLower(dev_type);
    if (DEV_TYPE_VAL_SOCKET == dev_type) {
        ret_dev = getSocketDeviceType(book, section);
        std::cout << "dev type is socket\n";
    }
    else if (DEV_TYPE_VAL_PIPE == dev_type) {
        ret_dev = getPipeDeviceType(book, section);
        std::cout << "dev type is fifo\n";
    }

    return static_cast<int>(ret_dev);
}

infra::EDeviceType DeviceTypeReader::getSocketDeviceType(const infra::config::ConfigurationBook & book,
                                                        std::string_view section)
{
    using namespace infra;
    using namespace infra::config;

    EDeviceType ret_dev{EDeviceType::E_UNDEFINED_DEVICE};
    std::string socket_dom;
    std::string socket_type;
    if (!(book.valueFor(ConfigurationAddress{section, SOCKET_TYPE_KEY}, socket_type))){
        std::cerr << "Could not find socket type key\n";
        return ret_dev;
    }
    if (!(book.valueFor(ConfigurationAddress{section, SOCKET_DOMAIN_KEY}, socket_dom))){
        std::cerr << "Could not find socket domain key\n";
        return ret_dev;
    }

    toLower(socket_dom);
    toLower(socket_type);

    std::cout << "socket domain is " << socket_dom << "\n";
    std::cout << "socket type is " << socket_type << "\n";

    if (auto sock_type_map_iter = SocketConfigInfoToType.find(socket_type);
            SocketConfigInfoToType.end() != sock_type_map_iter)
    {
        std::cout << "socket type found \n";
        auto & sock_domain_map_iter = sock_type_map_iter->second;
        if (auto domain_to_type_iter = sock_domain_map_iter.find(socket_dom);
                sock_domain_map_iter.end() != domain_to_type_iter)
        {
            ret_dev = domain_to_type_iter->second;
        }
        else
        {
            std::cerr << "no entry in map for device type\n";
        }
    }
    std::cout << "return dev type: " << ret_dev << "\n";
    return ret_dev;
}

infra::EDeviceType DeviceTypeReader::getPipeDeviceType(const infra::config::ConfigurationBook & book,
                                                      std::string_view section)
{
    using namespace infra;
    using namespace infra::config;

    EDeviceType ret_dev{EDeviceType::E_UNDEFINED_DEVICE};
    std::string profile;
    if (!(book.valueFor(ConfigurationAddress{section, PROFILE_KEY}, profile))){
        return ret_dev;
    }

    toLower(profile);
    if (WRITE_PROFILE_VAL == profile){
        ret_dev = EDeviceType::E_READING_FIFO_DEVICE;
    }
    else if(READ_PROFILE_VAL == profile){
        ret_dev = EDeviceType::E_WRITING_FIFO_DEVICE;
    }

    return ret_dev;
}

} //infra
