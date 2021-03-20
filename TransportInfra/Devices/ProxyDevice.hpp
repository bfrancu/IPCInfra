#ifndef PROXYDEVICE_HPP
#define PROXYDEVICE_HPP
#include <functional>
#include <optional>

#include "GenericDeviceAccess.hpp"
#include "Traits/device_constraints.hpp"
#include "default_traits.hpp"

namespace infra
{

template<typename BaseDevice>
class ProxyDevice : public traits::select_traits<BaseDevice>
{
public:
    using handle_type = typename BaseDevice::handle_type;

    void setBaseReference(BaseDevice & device){
        m_wrapped_device = std::ref(device);
    }

public:
    handle_type getHandle() const { 
        if (!m_wrapped_device.has_value()){
            return meta::traits::default_value<handle_type>::value;
        }
        return GenericDeviceAccess::getHandle(m_wrapped_device.value().get());
    }

protected:
    bool m_reference_set{false};
    std::optional<std::reference_wrapper<BaseDevice>> m_wrapped_device;
};
}//infra

#endif
