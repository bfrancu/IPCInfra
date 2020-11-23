#include <iostream>
#include <deque>
#include <unordered_set>

#include "ConfigurationBook.h"

struct StringWrapper
{
    StringWrapper() = default;

    StringWrapper(std::string val) :
        value(val)
    {}

    StringWrapper & operator=(std::string_view view_val)
    {
        value = view_val;
        return *this;
    }

    friend std::ostream & operator<<(std::ostream & os, const StringWrapper & wrapper)
    {
        os << wrapper.value;
        return os;
    }

    std::string value;
};

template<typename Collection>
void printCollection(const Collection & coll)
{
    for (auto & item : coll)
    {
        std::cout << item << " ";
    }
    std::cout << "\n";
}

int main()
{
    using namespace infra::config;
    std::string config_file{"/home/bfrancu/Documents/Work/Projects/IPCInfra/Configuration/example.ini"};
    ConfigurationBook book(config_file);
    if(book.init())
    {
        book.printTable();
        int service{0};
        StringWrapper host_wrapper;
        //std::string host_wrapper;

        if (book.valueFor(ConfigurationAddress("CONNECTION_DETAILS", "SERVICE"), service))
        {
            std::cout << "service is " << service << "\n";
        }
        else
        {
            std::cerr << "couldn't find service value\n";
        }

        if (book.valueFor(ConfigurationAddress("CONNECTION_DETAILS", "HOST"), host_wrapper))
        {
            std::cout << "host is " << host_wrapper << "\n";
        }
        else
        {
            std::cerr << "couldn't find host value\n";
        }

        std::vector<int> primes_col;
        ConfigurationAddress primes_cfg{"HONEY_BUNNY", "primes"};
        if (book.collectionFor(primes_cfg, primes_col, ", "))
        {
            printCollection(primes_col);
        }
        else
        {
            std::cerr << "couldn't extract primes\n";
        }

        std::deque<std::string> primates_col;
        ConfigurationAddress primates_cfg{"HONEY_BUNNY", "primates"};
        if (book.collectionFor(primates_cfg, primates_col))
        {
            printCollection(primates_col);
        }
        else
        {
            std::cerr << "couldn't extract primates\n";
        }
        
        std::unordered_set<std::string> friends_col;
        ConfigurationAddress friends_cfg{"CONNECTION_DETAILS", "FRIENDS"};
        if (book.collectionFor(friends_cfg, friends_col))
        {
            printCollection(friends_col);
        }
        else
        {
            std::cerr << "couldn't extract friends\n";
        }

    }

    return 0;
}
