#ifndef CONNECTOR_CLIENT_H
#define CONNECTOR_CLIENT_H
#include <string>
#include <unordered_map>


#include "Devices/DeviceDefinitions.h"

namespace infra
{
namespace config
{
   class ConfigurationBook;
}
}

class ConnectorClient
{
public:
    ConnectorClient(std::string_view file_name):
        config_file{file_name}
    {}

public:
    void init(std::string_view section);

protected:
    infra::EDeviceType getDeviceType(const infra::config::ConfigurationBook & book,
                                     std::string_view section);

    infra::EDeviceType getSocketDeviceType(const infra::config::ConfigurationBook & book,
                                     std::string_view section);

    infra::EDeviceType getPipeDeviceType(const infra::config::ConfigurationBook & book,
                                     std::string_view section);
private:
    using SockDomainToDevTypeMap = std::unordered_map<std::string, infra::EDeviceType>;
    using SockTypeMap = std::unordered_map<std::string, SockDomainToDevTypeMap>; 

    static SockTypeMap SocketConfigInfoToType;
    std::string config_file;
    
};

void toLower(std::string & value);

#endif
