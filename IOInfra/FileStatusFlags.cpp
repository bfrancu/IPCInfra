#include <fcntl.h>
#include <map>

#include "FileStatusFlags.h"

namespace infra
{

namespace io
{

namespace
{
std::map<EAccessMode, std::string> accessModeToStringMap{
    {EAccessMode::E_UNDEFINED,  "undefined"},
    {EAccessMode::E_READ_ONLY,  "read only"},
    {EAccessMode::E_WRITE_ONLY, "write only"},
    {EAccessMode::E_READ_WRITE, "read/write"},
    {EAccessMode::E_READ_WRITE, "append"}
};

std::map<EAccessMode, int> accessModeToFlagInt{
    {EAccessMode::E_READ_ONLY,  O_RDONLY},
    {EAccessMode::E_WRITE_ONLY, O_WRONLY},
    {EAccessMode::E_READ_WRITE, O_RDWR},
    {EAccessMode::E_APPEND,     O_WRONLY | O_APPEND}
};
}

FileStatusFlags::FileStatusFlags(file_descriptor_index descriptor) :
    m_descriptor {descriptor},
    m_access_mode {EAccessMode::E_UNDEFINED},
    m_flags{0}
{
    init();
}

void FileStatusFlags::init()
{   
    if (-1 == m_descriptor)
    {
        std::cerr << "FileStatusFlags::init invalid descriptor \n";
    }
    int flags = getLatestFlags();
    if (-1 != flags)
    {
        settAccessModeFlag(flags);
        if (valid())
        {
            setFlagsMask(flags);
        }
    }
    else
    {
        std::cerr << "FileStatusFlags::init fcntl(F_GETFL) failed with " << errno << "\n";
    }
}

bool FileStatusFlags::setFlagsMask(int flags)
{
    bool ret{false};

    if (valid() && m_flags != flags)
    {
        m_flags = flags;
        ret = true;
    }
    return ret;
}

void FileStatusFlags::settAccessModeFlag(int flags)
{
    int access_mode = flags & O_ACCMODE;

    if (O_RDONLY == access_mode)
    {
        m_access_mode = EAccessMode::E_READ_ONLY;
    }
    else if (O_WRONLY == access_mode)
    {
        m_access_mode = EAccessMode::E_WRITE_ONLY;
    }
    else if (O_RDWR == access_mode)
    {
        m_access_mode = EAccessMode::E_READ_WRITE;
    }
}

int FileStatusFlags::getLatestFlags()
{
    return fcntl(m_descriptor, F_GETFL);
}

bool FileStatusFlags::getSingleFlag(int flag) const
{
    bool ret{false};
    if (valid())
    {
        ret = (m_flags & flag);
    }
    return ret;
}

bool FileStatusFlags::setSingleFlag(int flag, bool requested)
{
    bool ret{false};
    if (!valid())
    {
        return ret;
    }

    bool current_value = getSingleFlag(flag);
    if (requested != current_value)
    {
        /*different bit operations for setting/unsetting a bit mask*/
        int changed_flags = requested ? m_flags | flag : m_flags & ~flag;
        /*change the flags with F_SETFL*/
        fcntl(m_descriptor, F_SETFL, changed_flags);

        /*get the latest flags with F_GETFL*/
        int latest_flags = fcntl(m_descriptor, F_GETFL);
        /*if they are different than the current flags - locally stored*/
        if (setFlagsMask(latest_flags))
        {
            /*check the set flags are the same as the latest*/
            ret = (changed_flags == latest_flags);
        }
    }

    return ret;
}

bool FileStatusFlags::setFlags(int value)
{
    bool ret{false};
    EAccessMode last_access_mode = m_access_mode;
    settAccessModeFlag(value);
    if (valid() && setFlagsMask(value))
    {
        ret = true;
    }
    else if (EAccessMode::E_UNDEFINED != last_access_mode)
    {
        m_access_mode = last_access_mode;
    }
    return ret;
}

bool FileStatusFlags::setAppend(bool value)
{
    return setSingleFlag(O_APPEND, value);
}


bool FileStatusFlags::setNoATime(bool value)
{
    return setSingleFlag(O_NOATIME, value);
}

bool FileStatusFlags::setDirect(bool value)
{
    return setSingleFlag(O_DIRECT, value);
}

bool FileStatusFlags::setAsync(bool value)
{
    return setSingleFlag(O_ASYNC, value);
}

bool FileStatusFlags::setNonBlock(bool value)
{
    return setSingleFlag(O_NONBLOCK, value);
}

int FileStatusFlags::getFlags() const
{
    return m_flags;
}

EAccessMode FileStatusFlags::accessMode() const
{
    return m_access_mode;
}

bool FileStatusFlags::append() const
{
    return getSingleFlag(O_APPEND);
}

bool FileStatusFlags::async() const
{
    return getSingleFlag(O_ASYNC);
}

bool FileStatusFlags::valid() const
{
    return (EAccessMode::E_UNDEFINED != m_access_mode ||
            m_descriptor >= 0);
}

bool FileStatusFlags::nonBlock() const
{
    return getSingleFlag(O_NONBLOCK);
}

bool FileStatusFlags::direct() const
{
    return getSingleFlag(O_DIRECT);
}

bool FileStatusFlags::noAtime() const
{
    return getSingleFlag(O_NOATIME);
}

bool FileStatusFlags::dsync() const
{
    return getSingleFlag(O_DSYNC);
}

bool FileStatusFlags::sync() const
{
    return getSingleFlag(O_SYNC);
}

std::ostream &operator<<(std::ostream &os, const FileStatusFlags &flags)
{
    os << "File status flags: "
       << "\nO_APPEND:   "  << std::boolalpha << flags.append()
       << "\nO_NONBLOCK: "  << flags.nonBlock()
       << "\nO_NOATIME:  "  << flags.noAtime()
       << "\nO_DIRECT:   "  << flags.direct()
       << "\nO_ASYNC:    "  << flags.async()
       << "\nO_DSYNC:    "  << flags.dsync()
       << "\nO_SYNC:     "  << flags.sync()
       << "\nAccess mode: " << accessModeToStringMap[flags.accessMode()] << "\n";

    return os;
}

int getAccessModeFlag(EAccessMode access_mode)
{
    int ret{0};
    if (EAccessMode::E_UNDEFINED != access_mode)
    {
        ret = accessModeToFlagInt[access_mode];
    }
    return ret;
}

} //io
} //infra
