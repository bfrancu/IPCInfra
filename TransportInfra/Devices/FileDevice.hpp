#ifndef FILEDEVICE_HPP
#define FILEDEVICE_HPP

#include <type_traits>
#include <string>

#include "FileIODefinitions.h"
#include "LinuxUtils/LinuxIOUtilities.h"

#include "Traits/device_traits.hpp"

namespace infra
{

template<typename ResourceHandler, typename = void>
class FileDevice{};


template<typename ResourceHandler>
class FileDevice<ResourceHandler, std::enable_if_t<std::conjunction_v<std::is_same_v<my_custom_platform, typename ResourceHandler::platform>,
                                                                      std::negation_v<HasUnixHandleTypeT<ResourceHandler>::value>>>>
{};


template<typename ResourceHandler>
class FileDevice<ResourceHandler, std::enable_if_t<HasUnixHandleTypeT<ResourceHandler>::value>>
{
    friend class GenericDeviceAccess;
public:
    using handle_type = typename handler_traits<ResourceHandler>::handle_type;
    using platform    = typename handler_traits<ResourceHandler>::platform;
    using sequential  = sequential_device;

public:
     FileDevice() :
         m_filename{NEW_FILE_NAME},
         m_directory{utils::unx::LinuxIOUtilities::getCurrentDir()}
     {}

     FileDevice(const std::string & pathname) :
         m_filename{},
         m_directory{utils::unx::LinuxIOUtilities::getCurrentDir()}
     {
         setDirFilePaths(pathname);
     }

     FileDevice(const FileDevice & other) :
         m_resource_handler{other.m_resource_handler},
         m_filename{other.m_filename},
         m_directory{other.m_directory}
     {}

     FileDevice(FileDevice && other) noexcept :
         m_resource_handler{std::move(other.m_resource_handler)},
         m_filename{std::move(other.m_filename)},
         m_directory{std::move(other.m_directory)}
     {}

     FileDevice & operator=(const FileDevice & other){
         if(this == &other) return *this;
         m_resource_handler = other.m_resource_handler;
         m_filename = other.m_filename;
         m_directory = other.m_directory;
         return *this;
     }

     FileDevice & operator=(FileDevice && other) noexcept{
         m_resource_handler = std::move(other.m_resource_handler);
         m_filename = std::move(other.m_filename);
         m_directory = std::move(other.m_directory);
         return *this;
     }

public:
    bool open(const std::string & pathname, io::EAccessMode access_mode = io::EAccessMode::E_READ_WRITE, int open_file_flags = 0){
        bool ret{false};
        if (setFileName(pathname)){
            ret = openFileImpl(access_mode, open_file_flags, 0);
        }
        return ret;
    }

    bool open(io::EAccessMode access_mode = io::EAccessMode::E_READ_WRITE, int open_file_flags = 0){
        return openFileImpl(access_mode, open_file_flags, 0);
    }

    void close(){ m_resource_handler.close(); }
    bool remove(){
        if (m_resource_handler.open()) {
            close();
        }
        bool ret{remove(getFullPath())};
        setDirFilePaths(NEW_FILE_NAME);
        return ret;
    }

    bool setFileName(const std::string &pathname){
        bool ret{false};
        if (!m_resource_handler.open()){
            setDirFilePaths(pathname);
            ret = true;           
        }
        return ret;
    }

    bool isOpen() const { return m_resource_handler.open(); }
    std::string getFileName() const { return m_filename; }
    std::string getFilePath() const { return m_directory; }
    std::string getFullPath() const { return utils::unx::LinuxIOUtilities::fullPath(m_filename, m_directory); }

protected:
    handle_type getHandle() const { return m_resource_handler.getHandle(); }

    void setDirFilePaths(const std::string &pathname){
        if (pathname != getFullPath()){
            auto dir_file_pair = utils::unx::LinuxIOUtilities::getDirectoryAndFileName(pathname);
            if (!dir_file_pair.first.empty()){
                m_directory = dir_file_pair.first;
            }
            m_filename = dir_file_pair.second.empty() ? NEW_FILE_NAME : dir_file_pair.second;
        }
    }

    bool openFileImpl(io::EAccessMode access_mode, int open_file_Flags, unsigned int mode_flags){
        handle_type local_handle{ResourceHandler::getDefaultValue()};
        std::string full_path = getFullPath();
        if (m_resource_handler.open()){
            return false;
        }

        if(utils::unx::LinuxIOUtilities::exists(full_path)){
            local_handle = utils::unx::LinuxIOUtilities::openExistingFile(full_path, access_mode, open_file_Flags);
        }
        else{
            local_handle = utils::unx::LinuxIOUtilities::openNewFile(full_path, access_mode, open_file_Flags, mode_flags);
        }

        if (local_handle != ResourceHandler::getDefaultValue()){
            m_resource_handler.acquire(local_handle);
            return true;
        }
        return false;
    }

protected:
     inline static const char * const NEW_FILE_NAME{"New File.txt"};

protected:
    ResourceHandler m_resource_handler;
    std::string m_filename;
    std::string m_directory;   

};
} // infra
#endif // FILEDEVICE_HPP
