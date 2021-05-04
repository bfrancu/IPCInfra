#ifndef PROXYDEVICE_HPP
#define PROXYDEVICE_HPP
#include <functional>
#include <optional>

#include "GenericDeviceAccess.hpp"
#include "Traits/device_constraints.hpp"
#include "default_traits.hpp"

//#include "randomizer.hpp"

namespace infra
{

template<typename BaseDevice>
class ProxyDevice : public traits::select_traits<BaseDevice>
{
public:
    using handle_type = typename BaseDevice::handle_type;

    void setBaseReference(BaseDevice & device){
        //std::cout << "ProxyDevice::setBaseReference() setting reference to device\n";
        m_wrapped_device = std::ref(device);

        //std::cout << "ProxyDevice::setBaseReference() device has value: " << m_wrapped_device.has_value() << "; id: " << id << "\n";

        //std::cout << "ProxyDevice::setBaseReference() handle: " << GenericDeviceAccess::getHandle(m_wrapped_device.value().get()) << "\n";
    }

public:
    handle_type getHandle() const {
        //std::cout << "ProxyDevice::getHandle() device has value: " << m_wrapped_device.has_value() << "; id: " << id << "\n";

        if (!m_wrapped_device.has_value()){
            //std::cout << "ProxyDevice::getHandle() wrapped device doesn't have value\n";
            return meta::traits::default_value<handle_type>::value;
        }
        return GenericDeviceAccess::getHandle(m_wrapped_device.value().get());
    }

protected:
    //bool m_reference_set{false};
    std::optional<std::reference_wrapper<BaseDevice>> m_wrapped_device;
    //int id{getRandomNumberInRange(0, 5000)};
};
}//infra

#endif
