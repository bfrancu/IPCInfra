#ifndef CONFIGURATION_READER_H
#define CONFIGURATION_READER_H
#include <string>
#include <unordered_map>
#include <vector>
#include <string_view>
#include <fstream>

#include "../UtilitiesInfra/shared_lookup_table.hpp"

namespace infra
{

using key_value_container = std::vector<std::pair<std::string_view, std::string_view>>;
//using section_table = infra::shared_lookup_table<std::string_view, key_value_container, std::unordered_map>;
using section_table = std::unordered_map<std::string_view, key_value_container>;

class ConfigurationReader
{
public:
    ConfigurationReader();
    ConfigurationReader(const std::string & file_name);

public:
    bool open(const std::string & file_name);
    bool readIn(std::string & result);
    bool fillTable(std::string_view content, section_table & out_table);

public:
    bool isOpen() const { return m_file_stream.is_open(); }

public:
    // TEST_ONLY
//    bool isReady() const { return !m_section_table.empty(); }

protected:

private:
    std::fstream m_file_stream;
};

std::streamoff streamSize(std::istream & f);
bool readContent(std::istream & f, std::string & result);

} // infra
#endif
