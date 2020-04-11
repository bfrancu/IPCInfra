#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <iostream>
#include <map>
#include <array>
#include <algorithm>
#include <iomanip> // defines putenv in POSIX

#include "FileInfo.h"

namespace infra
{

namespace io
{

namespace
{
std::map<EFileType, std::string> typeToStringMap{
    { EFileType::E_UNDEFINED,        "undefined"},
    { EFileType::E_REGULAR,          "regular"},
    { EFileType::E_DIRECTORY,        "directory"},
    { EFileType::E_CHARACTER_DEVICE, "character device"},
    { EFileType::E_BLOCK_DEVICE,     "block device"},
    { EFileType::E_LOCAL_SOCKET,     "socket"},
    { EFileType::E_NAMED_PIPE,       "FIFO or pipe"},
    { EFileType::E_SYMBOLIC_LINK,    "symbolic link"}
};
}

FileInfo::FileInfo(file_descriptor_index descriptor) :
    m_descriptor{descriptor},
    m_pathname{}
{
    init();
}

FileInfo::FileInfo(const std::string &pathname) :
    m_descriptor{-1},
    m_pathname{pathname}
{
    init();
}

void FileInfo::init()
{
    struct stat file_stats;
    int result = (-1 == m_descriptor) ? stat(m_pathname.c_str(), &file_stats) :
                                        fstat(m_descriptor, &file_stats);
    if (0 != result)
    {
        std::cerr << "FileInfo::init() fstat failed with " << errno << "\n";
        return;
    }
    setFields(file_stats);
}

void FileInfo::setFields(const struct stat & file_stats)
{
    m_available_size = file_stats.st_size;
    m_major_inode_dev = major(file_stats.st_dev);
    m_minor_inode_dev = minor(file_stats.st_dev);
    m_inode_number = file_stats.st_ino;
    m_user_id = file_stats.st_uid;
    m_group_id = file_stats.st_gid;

    m_type = getFileType(file_stats);
    m_last_access_time = getLocalTime(&file_stats.st_atime);
    m_last_modif_time = getLocalTime(&file_stats.st_mtime);
    m_last_status_change_time = getLocalTime(&file_stats.st_ctime);
    m_permissions.setFlags(file_stats.st_mode);
}

EFileType FileInfo::getFileType(const struct stat &file_stats)
{
    EFileType ret{EFileType::E_UNDEFINED};
    switch(file_stats.st_mode & S_IFMT)
    {
    case S_IFREG : ret = EFileType::E_REGULAR;
        break;
    case S_IFDIR:  ret = EFileType::E_DIRECTORY;
        break;
    case S_IFCHR:  ret = EFileType::E_CHARACTER_DEVICE;
        break;
    case S_IFBLK:  ret = EFileType::E_BLOCK_DEVICE;
        break;
    case S_IFLNK:  ret = EFileType::E_SYMBOLIC_LINK;
        break;
    case S_IFIFO:  ret = EFileType::E_NAMED_PIPE;
        break;
    case S_IFSOCK: ret = EFileType::E_LOCAL_SOCKET;
        break;
    default:       ret = EFileType::E_UNDEFINED;
    }
    return ret;
}

std::tm FileInfo::getLocalTime(const time_t *p_timer)
{
    return *std::localtime(p_timer);
}


int FileInfo::availableSize() const
{
    return m_available_size;
}

int FileInfo::deviceMajorNode() const
{
    return m_major_inode_dev;
}

int FileInfo::deviceMinorNode() const
{
    return m_minor_inode_dev;
}

int FileInfo::iNodeNo() const
{
    return m_inode_number;
}

int FileInfo::userId() const
{
    return m_user_id;
}

int FileInfo::groupId() const
{
    return m_group_id;
}

EFileType FileInfo::type() const
{
    return m_type;
}

FilePermissions FileInfo::permissions() const
{
    return m_permissions;
}

tm FileInfo::lastAccess() const
{
    return m_last_access_time;
}

tm FileInfo::lastModification() const
{
    return m_last_modif_time;
}

tm FileInfo::lastStatusChange() const
{
    return m_last_status_change_time;
}

std::ostream & operator<<(std::ostream &os, const FileInfo &file_info)
{
    std::string type_str = typeToStringMap[file_info.type()];
    std::string perm_str = file_info.permissions().toString();
    std::tm last_access_tm = file_info.lastAccess();
    std::tm last_modif_tm = file_info.lastModification();
    std::tm last_change_tm = file_info.lastStatusChange();

    os << "Device info:"
       << "\nType:                     "       << type_str
       << "\nI-node number:            "       << file_info.iNodeNo()
       << "\nDevice containing i-node: major=" << file_info.deviceMajorNode() << " minor=" << file_info.deviceMinorNode()
       << "\nOwnership:                UID="   << file_info.userId() << " GID=" << file_info.groupId()
       << "\nFile mode:                "       << perm_str
       << "\nAvailable size:           "       << file_info.availableSize() << " bytes"
       << "\nLast file access:         "       << std::put_time(&last_access_tm, "%c %Z")
       << "\nLast file modification:   "       << std::put_time(&last_modif_tm, "%c %Z")
       << "\nLast status change:       "       << std::put_time(&last_change_tm, "%c %Z") << "\n";

    return os;
}

} //io

} //infra
