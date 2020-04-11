#ifndef FILEINFO_H
#define FILEINFO_H
#include <array>
#include <ctime>

#include <FileIODefinitions.h>

#include <FilePermissions.h>

struct stat;

namespace infra
{

namespace io
{

class FileInfo
{
public:
    FileInfo(file_descriptor_index descriptor);
    FileInfo(const std::string & pathname);

public:
    int availableSize() const;
    int deviceMajorNode() const;
    int deviceMinorNode() const;
    int iNodeNo() const;
    int userId() const;
    int groupId() const;
    EFileType type() const;
    FilePermissions permissions() const;
    std::tm lastAccess() const;
    std::tm lastModification() const;
    std::tm lastStatusChange() const;

private:
    void init();
    void setFields(const struct stat & file_stats);
    EFileType getFileType(const struct stat & file_stats);
    std::tm getLocalTime(const time_t * p_timer);

private:
    file_descriptor_index m_descriptor;
    int m_available_size                {0};
    int m_major_inode_dev               {0};
    int m_minor_inode_dev               {0};
    int m_inode_number                  {0};
    int m_user_id                       {0};
    int m_group_id                      {0};

    EFileType m_type                    {EFileType::E_UNDEFINED};
    FilePermissions m_permissions       {};
    std::tm m_last_access_time          {};
    std::tm m_last_modif_time           {};
    std::tm m_last_status_change_time   {};
    std::string m_pathname;
};

std::ostream & operator<<(std::ostream & os, const FileInfo & file_info);

} //io

} // infra
#endif // FILEINFO_H
