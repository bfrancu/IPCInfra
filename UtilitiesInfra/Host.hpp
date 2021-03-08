#ifndef HOST_HPP
#define HOST_HPP
#include "template_typelist.hpp"
#include "policies_initializer.hpp"

namespace infra
{

template<typename Client,
         template<typename... > typename... Plugins>
class Host : public Client,
             public Plugins<Host<Client, Plugins...>, Client>...
{
    using ConcretePluginsTList = meta::tl::typelist<Plugins<Host<Client, Plugins...>, Client>...>;
public:
    using Client::Client;

    Host() : Client(), Plugins<Host, Client>()...
    {}

    template<typename... Args>
    bool init(Args&&... args)
    {
        return initDispatch<ConcretePluginsTList>(*this, std::forward<Args>(args)...);
    }
};

template<typename Client, typename TList>
struct PackHost;

template<typename Client,
         template <typename...> typename... Plugins>
struct PackHost<Client, meta::ttl::template_typelist<Plugins...>>
{
    using Type = Host<Client, Plugins...>;
};

template<typename Client, typename TList>
using PackHostT = typename PackHost<Client, TList>::Type;

template<typename PackedHost>
struct UnpackHost;

template<typename Client,
         template<typename...> typename... Plugins>
struct UnpackHost<Host<Client, Plugins...>>
{
    using ClientT = Client;
    using PluginsTList = meta::ttl::template_typelist<Plugins...>;
};

template<typename PackedHost>
using UnpackedClientT = typename UnpackHost<PackedHost>::ClientT;

template<typename PackedHost>
using UnpackedPluginsTList = typename UnpackHost<PackedHost>::PluginsTList;

} //infra
#endif // HOST_HPP
