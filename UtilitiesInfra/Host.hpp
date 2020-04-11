#ifndef HOST_HPP
#define HOST_HPP

namespace infra
{

template<typename Device,
         template<typename... > typename... Policies>
class Host : public Device,
             public Policies<Host<Device, Policies...>, Device>...
{
public:
    using Device::Device;
    using Device::getHandle;

    Host() : Device(), Policies<Host, Device>()...
    {}

};
} //io
#endif // HOST_HPP
