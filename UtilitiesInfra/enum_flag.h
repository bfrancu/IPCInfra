#ifndef ENUM_FLAG_H
#define ENUM_FLAG_H

#include <type_traits>

namespace infra
{
template <typename T>
class enum_flag
{
public:
    /*
     * use underlying_type to constraint the use just for enums
     */
    enum_flag(T enum_value)
    {
        m_flag_value = static_cast<typename std::underlying_type<T>::type>(enum_value);
    }

    operator int() const
    {
        return  m_flag_value;
    }

    /*
    operator bool() const
    {
        return 0 != m_flag_value;
    }
    */

    enum_flag<T> operator &(const enum_flag<T> & other) const
    {
        /*copy construct a new enum_flag from the current instance as there is no default constructor*/
        enum_flag<T> ret{*this};
        /*overwrite the return m_flag_value with the operation result*/
        ret.m_flag_value = m_flag_value & other.m_flag_value;
        return ret;
    }

    enum_flag<T> operator |(const enum_flag<T> & other) const
    {
        enum_flag<T> ret{*this};
        ret.m_flag_value = m_flag_value | other.m_flag_value;
        return ret;
    }

    enum_flag<T> operator ^(const enum_flag<T> & other) const
    {
        enum_flag<T> ret{*this};
        ret.m_flag_value = m_flag_value ^ other.m_flag_value;
        return ret;
    }

    enum_flag<T> operator <<(int shifts) const
    {
        enum_flag<T> ret{*this};
        ret.m_flag_value = m_flag_value << shifts;
        return ret;
    }

    enum_flag<T> operator >>(int shifts) const
    {
        enum_flag<T> ret{*this};
        ret.m_flag_value = m_flag_value >> shifts;
        return ret;
    }

    enum_flag<T> operator ~() const
    {
        enum_flag<T> ret{*this};
        ret.m_flag_value = ~m_flag_value;
        return ret;
    }

    enum_flag<T> operator &=(const enum_flag<T> & other)
    {
        m_flag_value &= other.m_flag_value;
        return *this;
    }

    enum_flag<T> operator |=(const enum_flag<T> & other)
    {
        m_flag_value |= other.m_flag_value;
        return *this;
    }

    enum_flag<T> operator ^=(const enum_flag<T> & other)
    {
        m_flag_value ^= other.m_flag_value;
        return *this;
    }

    enum_flag<T> operator <<=(int shifts)
    {
        m_flag_value <<= shifts;
        return *this;
    }

    enum_flag<T> operator >>=(int shifts)
    {
        m_flag_value >>= shifts;
        return *this;
    }

private:
    int m_flag_value;
};

} //infra

#endif // ENUM_FLAG_H
