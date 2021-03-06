#ifndef CONFIGURATION_BOOK_H
#define CONFIGURATION_BOOK_H
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <shared_mutex>

#include "ConfigurationReader.h"
#include "ConversionUtils.h"

namespace infra
{

namespace config
{

struct ConfigurationAddress
{
    ConfigurationAddress() = default;
    ConfigurationAddress(std::string_view table_section, std::string_view section_key) :
        section(table_section),
        key(section_key)
    {}

    std::string_view section;
    std::string_view key;
};

//TODO ADD MULTITHREADED SUPPORT
class ConfigurationBook
{
public:
    ConfigurationBook();
    ConfigurationBook(const std::string & file_name);

public:
    bool init();
    bool initFrom(const std::string & file_name);
    bool isReady() const;
    void printTable();

public:
    template<typename T>
    bool valueFor(const ConfigurationAddress & addr, T & out_value) const;

    template<typename Container, typename = std::void_t<typename Container::value_type>>
    bool collectionFor(const ConfigurationAddress & addr, Container & out_container, std::string_view sep = DEFAULT_SEP) const;

protected:
    bool findValueInTable(const ConfigurationAddress & addr, std::string_view & out_value) const;

protected:
    static constexpr const char * DEFAULT_SEP{" "};

private:
    mutable std::shared_mutex m_mutex;
    std::string m_file_content;
    ConfigurationReader m_reader;
    section_table m_section_table;
};

} //config
} //infra

#include "ConfigurationBook.hpp"
#endif
