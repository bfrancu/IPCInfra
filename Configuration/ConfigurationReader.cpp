#include <iostream>
#include <functional>
#include "ConfigurationReader.h"

namespace
{
const std::string OPEN_SECTION_SEP {"["};
const std::string CLOSE_SECTION_SEP {"]"};
const std::string KEY_VALUE_SEP{"="};
const std::string ENDLINE_SEP{'\n'};
}

namespace infra
{

namespace config
{

ConfigurationReader::ConfigurationReader() :
    m_file_stream()
{}

ConfigurationReader::ConfigurationReader(const std::string & file_name) :
    m_file_stream(file_name)
{}

bool ConfigurationReader::open(const std::string & file_name)
{
    if (m_file_stream.is_open()){
        m_file_stream.close();
    }
    m_file_stream.open(file_name);
    return !m_file_stream.fail();
}

/*bool ConfigurationReader::init()
{
    if(readContent(m_file_stream, m_file_content) &&
        fillTable(m_file_content, m_section_table)){
        return true;
    }
    return false;
}
*/
bool ConfigurationReader::readIn(std::string & result)
{
    if (!isOpen()) return false;
    return readContent(m_file_stream, result);
}

bool ConfigurationReader::fillTable(std::string_view content, section_table &out_table)
{
    static std::boyer_moore_horspool_searcher section_open_searcher{std::begin(OPEN_SECTION_SEP), std::end(OPEN_SECTION_SEP)};
    static std::boyer_moore_horspool_searcher section_close_searcher{std::begin(CLOSE_SECTION_SEP), std::end(CLOSE_SECTION_SEP)};
    static std::boyer_moore_horspool_searcher key_value_sep_searcher{std::begin(KEY_VALUE_SEP), std::end(KEY_VALUE_SEP)};
    static std::boyer_moore_horspool_searcher endline_searcher{std::begin(ENDLINE_SEP), std::end(ENDLINE_SEP)};

    if (!out_table.empty()) out_table.clear();

    // search for [
    for (auto [section_open_pos, after_section_open_pos] = section_open_searcher(content.begin(), content.end());
         section_open_pos != content.end();)
    {
        // search for the next ]
        auto [section_close_pos, after_section_close_pos] = section_close_searcher(after_section_open_pos, content.end());
        if (content.end() == section_close_pos) break;

        // get section name in between []
        std::string_view section_name = content.substr(std::distance(content.begin(), after_section_open_pos),
                                                       std::distance(after_section_open_pos, section_close_pos));

        // get the first key value separator =
        auto [key_value_sep_pos, after_key_value_sep_pos] = key_value_sep_searcher(after_section_close_pos, content.end());
        // search for the next [ in order to delimit this section
        std::tie(section_open_pos, after_section_open_pos) = section_open_searcher(after_section_open_pos, content.end());

        // inseart in the map an empty container for the section_name key
        //out_table.add_or_update_mapping(section_name, key_value_container());

        out_table[section_name] = key_value_container();
        std::cout << "Section name: " << section_name << "\n";
        std::cout << "\n\n";

        std::size_t pairs_count = std::count(key_value_sep_pos, section_open_pos, KEY_VALUE_SEP[0]);
        if (pairs_count < out_table[section_name].max_size()) out_table[section_name].reserve(pairs_count);

        while(key_value_sep_pos < section_open_pos)
        {
            // the key should start right after the previous endline
            auto [next_line_pos, after_next_line_pos] = endline_searcher(after_key_value_sep_pos, content.end());
            auto [after_prev_line_pos, prev_line_pos] = [key_value_sep_pos, &content](){
                auto [pos, next_pos] = endline_searcher(std::make_reverse_iterator(key_value_sep_pos), content.rend());
                return std::make_pair(pos.base(), next_pos.base());
            }();

            //auto iNext_line_pos = std::distance(content.begin(), next_line_pos);
            //auto iAfter_NextLine_pos = std::distance(content.begin(), after_next_line_pos);
            //auto iPrev_line_pos = std::distance(content.begin(), prev_line_pos);
            //auto iAfter_Prev_line_pos = std::distance(content.begin(), after_prev_line_pos);
            //std::cout << "next line pos: " << iNext_line_pos << ", after next line pos: " << iAfter_NextLine_pos << "\n"
            //         << "prev line pos: " << iPrev_line_pos << ", afte prev line pos: " << iAfter_Prev_line_pos << "\n";

            std::string_view key = content.substr(std::distance(content.begin(), after_prev_line_pos),
                                                  std::distance(after_prev_line_pos, key_value_sep_pos));

            std::string_view value = content.substr(std::distance(content.begin(), after_key_value_sep_pos),
                                                    std::distance(after_key_value_sep_pos, next_line_pos));

            std::cout << "key=" << key << "; value=" << (value) << "\n";

            out_table[section_name].emplace_back(std::make_pair(key,value));
            std::tie(key_value_sep_pos, after_key_value_sep_pos) = key_value_sep_searcher(after_next_line_pos, content.end());
        }
    }

    return !out_table.empty();
}


std::streamoff streamSize(std::istream & f)
{
    std::istream::pos_type current_pos = f.tellg();
    if (-1 == current_pos) return  -1;

    f.seekg(0, std::istream::end);
    std::istream::pos_type end_pos = f.tellg();
    f.seekg(current_pos);
    return end_pos - current_pos;
}

bool readContent(std::istream &f, std::string &result)
{
    auto stream_sz = streamSize(f);
    if (-1 == stream_sz || f.fail()){
        return false;
    }
    result.clear();
    result.resize(static_cast<typename std::string::size_type>(stream_sz));

    f.read(&result[0], stream_sz);
    return true;
}

} //config
} //infra
