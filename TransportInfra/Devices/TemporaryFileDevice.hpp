#ifndef TEMPORARYFILEDEVICE_HPP
#define TEMPORARYFILEDEVICE_HPP
#include "FileDevice.hpp"
#include "LinuxUtils/LinuxIOUtilities.h"
#include "FilePermissions.h"
#include "FileStatusFlags.h"

namespace infra
{

template<typename ResourceHandler, typename = std::void_t<>>
class TemporaryFileDevice : public FileDevice<ResourceHandler>
{};

template<typename ResourceHandler>
class TemporaryFileDevice<ResourceHandler,
                          std::void_t<std::enable_if_t<HasUnixHandleTypeT<ResourceHandler>::value>>>
        : public FileDevice<ResourceHandler>
{
    using Base = FileDevice<ResourceHandler>;
public:
    TemporaryFileDevice() :
        Base(){
        init(TEMP_PATH);
    }

    TemporaryFileDevice(const std::string & directory_name) :
        Base(){
        init(directory_name);
    }

    ~TemporaryFileDevice() { if(m_auto_remove) Base::remove();}

public:
    void setAutoRemove(bool auto_remove){ m_auto_remove = auto_remove; }
    bool autoRemove() const { return m_auto_remove; }

    bool saveAs(const std::string & pathname){
        bool ret{false};
        bool to_reopen{false};
        int current_open_status_flags{0};
        auto [directory, filename] = utils::unx::LinuxIOUtilities::getDirectoryAndFileName(pathname);

     /*chech if the given pathname is an absolute path or a filename*/
       std::string saved_file_path = directory.empty() ? utils::unx::LinuxIOUtilities::fullPath(filename, Base::getFilePath())
                                                       : pathname;

       if (Base::isOpen()){
            to_reopen = true;
            current_open_status_flags = io::FileStatusFlags(Base::getHandle()).getFlags();
            Base::close();
        }

        ret = utils::unx::LinuxIOUtilities::copyTo(Base::getFullPath(), saved_file_path);
        if (to_reopen) ret = Base::open(io::EAccessMode::E_READ_WRITE, current_open_status_flags);

        return ret;
    }

private:
   void init(const std::string & directory_name)
   {
       std::string file_name = utils::unx::LinuxIOUtilities::generateTempFileName(TEMP_FILE_LENGTH);
       directory_name.empty() ? Base::setDirFilePaths(utils::unx::LinuxIOUtilities::fullPath(file_name, TEMP_PATH))
                              : Base::setDirFilePaths(utils::unx::LinuxIOUtilities::fullPath(file_name, directory_name));
       io::FilePermissions temp_file_perm;
       temp_file_perm.set(io::EFilePermission::E_USER_READ, true);
       temp_file_perm.set(io::EFilePermission::E_USER_WRITE, true);

       Base::openFileImpl(io::EAccessMode::E_READ_WRITE, temp_file_perm.getModeFlags(), 0);
   }

private:
   inline static constexpr int TEMP_FILE_LENGTH{8};
   inline static constexpr char const * TEMP_PATH{"/tmp/"};

private:
   bool m_auto_remove {true};
};

} // infra


#endif // TEMPORARYFILEDEVICE_HPP
