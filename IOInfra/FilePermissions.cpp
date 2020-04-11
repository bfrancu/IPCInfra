#include <sys/stat.h>
#include <iostream>
#include <algorithm>

#include "FilePermissions.h"

namespace infra
{

namespace io
{

std::array<int, static_cast<int>(EFilePermission::E_LAST)> FilePermissions::mode_flag_array{
    S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};

using arr_size_t = std::array<bool, EFilePermission::E_LAST>::size_type;

FlagsArray FilePermissions::modeFlags()
{
    return mode_flag_array;
}

FilePermissions::FilePermissions() :
    m_flags{0}
{
   fillArray(m_flags);
}

FilePermissions::FilePermissions(unsigned int mode_flags) :
    m_flags{mode_flags}
{
    fillArray(m_flags);
}

FilePermissions::FilePermissions(std::initializer_list<EFilePermission> init_list) :
    m_flags{0}
{
    fillArray(m_flags);
    setInitialPermissions(init_list);
}

unsigned int FilePermissions::getModeFlags() const
{
    return m_flags;
}

void FilePermissions::setFlags(unsigned int mode_flags)
{
    if (m_flags != mode_flags)
    {
        m_flags = mode_flags;
    }
    fillArray(m_flags);
}

void FilePermissions::set(EFilePermission flag_index, bool value)
{
    std::cout << "FilePermissions::set() index = " << flag_index << "; value = " << std::boolalpha << value << "\n";
    if (flag_index < EFilePermission::E_LAST && value != m_array[static_cast<arr_size_t>(flag_index)])
    {        
        m_array[static_cast<arr_size_t>(flag_index)] = value;
        std::cout << "FilePermissions::set() flags = " << m_flags << "\n";
        m_flags = m_array[static_cast<arr_size_t>(flag_index)]
                ? m_flags | mode_flag_array[static_cast<arr_size_t>(flag_index)]
                : m_flags & ~mode_flag_array[static_cast<arr_size_t>(flag_index)];

        std::cout << "FilePermissions::set() flags = " << m_flags << "\n";
    }

}

std::string FilePermissions::toString() const
{
    std::string ret;
    for(size_t index = 0; index < m_array.size(); index ++)
    {
        std::string printable_permission;
        if (EFilePermission::E_USER_READ == index || EFilePermission::E_GROUP_READ == index
                || EFilePermission::E_OTHER_READ == index)
        {
            printable_permission = m_array[index] ? "r" : "-";
            ret.append(printable_permission);
        }
        else if (EFilePermission::E_USER_WRITE == index || EFilePermission::E_GROUP_WRITE == index
                 || EFilePermission::E_OTHER_WRITE == index)
        {
            printable_permission = m_array[index] ? "w" : "-";
            ret.append(printable_permission);
        }
        else if (EFilePermission::E_USER_EXECUTE == index || EFilePermission::E_GROUP_EXECUTE == index
                 || EFilePermission::E_OTHER_EXECUTE == index)
        {
            printable_permission = m_array[index] ? "x" : "-";
            ret.append(printable_permission);
        }
    }
    return ret;
}

bool FilePermissions::operator[](int index) const
{
    return m_array[static_cast<arr_size_t>(index)];
}

FilePermissions::operator unsigned int()
{
    std::cout << "FilePermissions::operator int()\n";
    return m_flags;
}

void FilePermissions::fillArray(unsigned int flags)
{
    for (size_t index = 0; index < mode_flag_array.size(); index++)
    {
        m_array[index] = flags & mode_flag_array[index];
    }
}

void FilePermissions::setInitialPermissions(const std::initializer_list<EFilePermission> &init_list)
{
    for (auto initial_permission : init_list)
    {
        set(initial_permission, true);
    }
}

} //io

} //infra
