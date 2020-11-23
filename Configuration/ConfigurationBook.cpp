#include <iostream>
#include <mutex>

#include "ConfigurationBook.h"

namespace infra
{

namespace config
{

ConfigurationBook::ConfigurationBook() :
    m_mutex{},
    m_file_content{},
    m_reader{},
    m_section_table{}
{}

ConfigurationBook::ConfigurationBook(const std::string & file_name) :
    m_mutex{},
    m_file_content{},
    m_reader{file_name},
    m_section_table{}
{}

bool ConfigurationBook::init()
{
    std::lock_guard<std::shared_mutex> lck{m_mutex};
    if(!m_reader.isOpen()) return false;

    if(m_reader.readIn(m_file_content))
    {
        return m_reader.fillTable(m_file_content, m_section_table);
    }
    return false;
}

bool ConfigurationBook::initFrom(const std::string & file_name)
{
    std::unique_lock<std::shared_mutex> lck{m_mutex};
    if(m_reader.open(file_name))
    {
        lck.unlock();
        return init();
    }
    return false;
}

bool ConfigurationBook::isReady() const
{
    std::shared_lock lck{m_mutex};
    return !m_section_table.empty();
}

void ConfigurationBook::printTable()
{
    std::shared_lock lck{m_mutex};
    for (auto it = m_section_table.begin(); it != m_section_table.end(); it++)
    {
        std::cout << "Section: " << it->first << "\n";
        std::for_each(it->second.begin(), it->second.end(), [](const auto & pair)
                      { std::cout << pair.first << "=" << pair.second << "\n"; });
    }
}

bool ConfigurationBook::findValueInTable(const ConfigurationAddress & addr, std::string_view & out_value) const
{
    std::shared_lock lck{m_mutex};
    if (auto section_it = m_section_table.find(addr.section);
            m_section_table.end() != section_it)
    {
        const key_value_container & container = section_it->second;
        auto container_it = std::find_if(container.cbegin(), container.cend(),[&addr] (const auto & pair){
                return pair.first == addr.key;});
        if (container.cend() != container_it)
        {
            out_value = container_it->second;
            std::cout << "ConfigurationBook::findValueInTable found: " << out_value << "\n";
            return true;
        }
    }

    std::cout << "ConfigurationBook::findValueInTable not found \n";
    return false;
}

}//config
} //infra
