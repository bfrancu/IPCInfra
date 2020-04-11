#ifndef FILESTATUSFLAGS_H
#define FILESTATUSFLAGS_H

#include <iostream>
#include <FileIODefinitions.h>

namespace infra
{

namespace io
{

class FileStatusFlags
{

public:
    FileStatusFlags(file_descriptor_index descriptor);

public:
    /*universal setter for all modifiable flags*/
    bool setFlags(int flags);

    /* Setters for modifiable open file status flags
     * We can use the fcntl() F_SETFL command to modify some of the open file status flags.
     * The flags that can be modified are O_APPEND, O_NONBLOCK, O_NOATIME, O_ASYNC, and
     * O_DIRECT. Attempts to modify other flags are ignored. (Some other UNIX imple-
     * mentations allow fcntl() to modify other flags, such as O_SYNC.)
     */
    /* Writes are always appended to the end of the file.*/
    bool setAppend(bool value);
    /*
     * Don’t update the file last access time (the st_atime field described in Sec-
     * tion 15.1) when reading from this file. To use this flag, the effective user
     * ID of the calling process must match the owner of the file, or the process
     * must be privileged (CAP_FOWNER); otherwise, open() fails with the error EPERM.
     * (In reality, for an unprivileged process, it is the process’s file-system user
     * ID, rather than its effective user ID, that must match the user ID of the file
     * when opening a file with the O_NOATIME flag, as described in Section 9.5.)
     * This flag is a nonstandard Linux extension. To expose its definition from
     * <fcntl.h>, we must define the _GNU_SOURCE feature test macro. The O_NOATIME
     * flag is intended for use by indexing and backup programs. Its use can sig-
     * nificantly reduce the amount of disk activity, because repeated disk seeks
     * back and forth across the disk are not required to read the contents of a
     * file and to update the last access time in the file’s i-node (Section 14.4).
     * Functionality similar to O_NOATIME is available using the MS_NOATIME mount()
     * flag (Section 14.8.1) and the FS_NOATIME_FL flag (Section 15.5).
     */
    bool setNoATime(bool value);
    /*
     * Allow file I/O to bypass the buffer cache. This feature is described in Sec-
     * tion 13.6. The _GNU_SOURCE feature test macro must be defined in order to
     * make this constant definition available from <fcntl.h>.
     */
    bool setDirect(bool value);
    /*
     * Generate a signal when I/O becomes possible on the file descriptor
     * returned by open(). This feature, termed signal-driven I/O, is available only
     * for certain file types, such as terminals, FIFOs, and sockets. (The O_ASYNC
     * flag is not specified in SUSv3; however, it, or the older synonym, FASYNC , is
     * found on most UNIX implementations.) On Linux, specifying the O_ASYNC
     * flag when calling open() has no effect. To enable signal-driven I/O, we must
     * instead set this flag using the fcntl() F_SETFL operation (Section 5.3). (Sev-
     * eral other UNIX implementations behave similarly.) Refer to Section 63.3
     * for more information about the O_ASYNC flag.
     */
    bool setAsync(bool value);

    /* Opens the file in nonblocking mode*/
    bool setNonBlock(bool value);

public:
    int getFlags() const;
    EAccessMode accessMode() const;

    bool append() const;
    bool async() const;
    bool nonBlock() const;
    bool direct() const;
    bool noAtime() const;
    bool dsync() const;
    /* all writes to the file are flushed to disk when opened with O_SYNC
     * Using the O_SYNC flag can strongly affect performance. See table 13-3
     */
    bool sync() const;

public:
    bool valid() const;

private:
    void init();
    bool setFlagsMask(int flags);
    void settAccessModeFlag(int flags);

    int getLatestFlags();

    /*set a single flag*/
    bool getSingleFlag(int flag) const;
    /*get a single flag*/
    bool setSingleFlag(int flag, bool requested);

private:
    file_descriptor_index m_descriptor;
    EAccessMode m_access_mode;
    int m_flags;
};

int getAccessModeFlag(EAccessMode access_mode);
std::ostream &operator<<(std::ostream & os, const FileStatusFlags & flags);

} //io

} //infra
#endif // FILESTATUSFLAGS_H
