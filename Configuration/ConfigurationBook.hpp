#ifndef CONFIGURATION_BOOK_HPP
#define CONFIGURATION_BOOK_HPP
#include <shared_mutex>

namespace infra
{
namespace config
{
    template<typename T>
        bool ConfigurationBook::valueFor(const ConfigurationAddress & addr, T & out_value) const
        {
            std::cout << "ConfigurationBook::valueFor() section: " << addr.section << " key: " << addr.key << "\n";
            std::string_view in_value;
            if (!findValueInTable(addr, in_value)){
                return false;
            }

            return utils::conversion::Helper<T>::convert(in_value, out_value);
        }


    template<typename Container, typename>
        bool ConfigurationBook::collectionFor(const ConfigurationAddress & addr, Container & out_container, std::string_view sep) const
        {
            using T = typename Container::value_type;
            std::string_view str_collection;
            bool res{false};
            if (!findValueInTable(addr, str_collection)){
                return false;
            }

            std::boyer_moore_horspool_searcher bm_searcher{sep.begin(), sep.end()};
            auto substr_start_pos = str_collection.begin();
            for (auto [substr_end_pos, after_substr_end_pos] = bm_searcher(substr_start_pos, str_collection.end());
                    substr_start_pos != str_collection.end();)
            {
                std::string_view str_element = str_collection.substr(std::distance(str_collection.begin(), substr_start_pos),
                        std::distance(substr_start_pos, substr_end_pos));
                T val_element;
                if (utils::conversion::Helper<T>::convert(str_element, val_element)){
                    out_container.insert(std::cend(out_container), std::move(val_element));
                    res = true;
                }

                substr_start_pos = after_substr_end_pos;
                std::tie(substr_end_pos, after_substr_end_pos) = bm_searcher(substr_start_pos, str_collection.end());
            }

            return res;
        }
    
}//config
}//infra

#endif
