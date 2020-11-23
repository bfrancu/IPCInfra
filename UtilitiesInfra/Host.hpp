#ifndef HOST_HPP
#define HOST_HPP

namespace infra
{

/* TODO
 * Refactor second Client template argument passed to Plugins
 * Because I used crtp_base for my policies it introduced an unnecessary constraint
 * in the higher level abstraction
 *
 */
template<typename Client,
         template<typename... > typename... Plugins>
class Host : public Client,
             public Plugins<Host<Client, Plugins...>, Client>...
{
public:
    using Client::Client;

    Host() : Client(), Plugins<Host, Client>()...
    {}
};

} //infra
#endif // HOST_HPP
