#ifndef RESOURCEHANDLER_HPP
#define RESOURCEHANDLER_HPP
#include "Traits/handler_traits.hpp"


namespace infra
{

class UnixResourceHandler
{
public:
    using handle_type = int;
    using platform = unix_platform;

public:
    UnixResourceHandler(handle_type handle = DEFAULT_VALUE);

    UnixResourceHandler(const UnixResourceHandler & other);
    UnixResourceHandler(UnixResourceHandler && other) noexcept;

    UnixResourceHandler & operator=(const UnixResourceHandler & other);
    UnixResourceHandler & operator=(UnixResourceHandler && other) noexcept;

    ~UnixResourceHandler();

public:
    handle_type getHandle() const { return m_handle; }

    handle_type release(){
        handle_type temp = m_handle;
        m_handle = DEFAULT_VALUE;
        m_open = false;
        return temp;
    }

    static handle_type getDefaultValue() { return DEFAULT_VALUE; }

    void acquire(handle_type handle);    
    void close();

    bool validHandle() const;
    bool open() const{ return m_open; }

protected:
    bool defaultHandle() const;

private:
    inline constexpr static handle_type DEFAULT_VALUE{handler_traits<UnixResourceHandler>::defaultValue()};

private:
    handle_type m_handle;
    bool m_open;
};

} // infra
#endif // RESOURCEHANDLER_HPP
