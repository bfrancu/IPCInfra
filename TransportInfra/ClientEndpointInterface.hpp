#ifndef CLIENT_ENDPOINT_IF_HPP
#define CLIENT_ENDPOINT_IF_HPP
#include "runtime_dispatcher.hpp"

template<typename Derived, typename TList>
class ClientEndpointInterface
{

};

class ITransportEndpoint
{
public:
    virtual ~ITransportEndpoint();
    void *getDevice();
};

template<typename ClientInterface, typename TList>
class DynamicInterfaceEnpointAdaptor : public ClientInterface
{
public:
    DynamicInterfaceEnpointAdaptor(ITransportEndpoint *adaptor) :
        m_pAdaptor(adaptor),
        ClientInterface(m_pAdaptor)
    {}

private:
    ITransportEndpoint *m_pAdaptor;
};

#endif
