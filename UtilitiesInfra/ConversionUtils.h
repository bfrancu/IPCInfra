#ifndef CONVERSION_UTILS_H
#define CONVERSION_UTILS_H

#include <type_traits>
#include <string_view>
#include <string>
#include <charconv>

namespace infra
{
namespace utils
{
namespace conversion
{
class ConversionUtilities
{
public:
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>>>>
    static bool toStringInBuffer(T to_convert, char *p_buf, std::size_t &len)
    {
        char *p_begin{p_buf};
        char *p_end{p_buf + len};
        auto [p_past_end, ec] = std::to_chars(p_begin, p_end, to_convert);

        if (std::errc() == ec){
            len = std::distance(p_begin, p_past_end);
            return true;
        }
        return false;
    }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>>>>
    static std::string toString(T to_convert)
    {
        std::string ret{};
        char buf[ARITHMETIC_BUFFER_LEN];
        std::fill(buf, buf + ARITHMETIC_BUFFER_LEN, 0);
        char *p_begin = buf;
        char *p_end = p_begin + ARITHMETIC_BUFFER_LEN;
        auto [p_past_end, ec] = std::to_chars(p_begin, p_end, to_convert);

        if (std::errc() == ec){
            ret = std::string(const_cast<const char*>(buf));
        }
        return ret;
    }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>>>>
    static T toArithmetic(std::string_view buf_view)
   {
        T converted_val;
        auto [p, ec] = std::from_chars(buf_view.data(), buf_view.data() + buf_view.size(), converted_val);
        if (std::errc() == ec){
            return converted_val;
        }

        return 0;
    }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>>>>
    static bool toArithmetic(std::string_view buf_view, T & converted_val)
    {
        auto [p, ec] = std::from_chars(buf_view.data(), buf_view.data() + buf_view.size(), converted_val);
        if (std::errc() == ec){
            return true;
        }
        return false;
    }

protected:
    static constexpr std::size_t ARITHMETIC_BUFFER_LEN{15};

}; //ConversionUtilities


template<typename T, typename = void>
struct Helper
{
   static constexpr bool convert(std::string_view value, T & out_value){
       (void) value;
       (void) out_value;
       return false;
   }
};

template<typename T>
struct Helper<T, std::enable_if_t<std::is_arithmetic_v<std::decay_t<T>>>>
{
    static constexpr bool convert(std::string_view value, T & out_value){
        return ConversionUtilities::toArithmetic(value, out_value);
    }
};

template<typename T>
struct Helper<T, std::enable_if_t<
                             std::conjunction_v<
                                 std::is_assignable<std::decay_t<T>, std::string_view>,
                                 std::negation<std::is_arithmetic<std::decay_t<T>>>>
                             >>
{
   static constexpr bool convert(std::string_view value, T & out_value){
       out_value = value;
       return true;
   }
};

} //conversion
} //utils
} //infra
#endif
