#ifndef CRTP_BASE_HPP
#define CRTP_BASE_HPP

namespace infra
{

template<typename Policy, typename Derived>
class crtp_base
{
protected:
    Derived & asDerived() { return *static_cast<Derived*>(this); }
    const Derived & asDerived() const { return *static_cast<Derived const *>(this); }
};

} //infra

#endif // CRTP_BASE_HPP
