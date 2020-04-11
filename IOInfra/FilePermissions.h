#ifndef FILEPERMISSIONS_H
#define FILEPERMISSIONS_H

#include <initializer_list>
#include <functional>
#include <array>

#include "FileIODefinitions.h"

namespace infra
{

namespace io
{

using FlagsArray = std::array<int, static_cast<int>(EFilePermission::E_LAST)>;

class FilePermissions
{
public:
    static FlagsArray modeFlags();

public:
    /*TODO Initializer list constructor*/
    FilePermissions();
    FilePermissions(unsigned int mode_flags);
    FilePermissions(std::initializer_list<EFilePermission> init_list);

    void setFlags(unsigned int mode_flags);
    void set(EFilePermission flag_index, bool value);
    unsigned getModeFlags() const;
    std::string toString() const;
    /*this should be used with the EFilePermission values as indexes*/
    bool operator[](int index) const;
    // this needs to be tested more
//    ValueChangedUpdater<bool> & operator[](int index);
    operator unsigned int();

private:
    void fillArray(unsigned int flags);
    void setInitialPermissions(const std::initializer_list<EFilePermission> & init_list);

private:
    static FlagsArray mode_flag_array;

private:
    //std::array<ValueChangedUpdater<bool>, EFilePermission::E_LAST> m_array;
    std::array<bool, EFilePermission::E_LAST> m_array;
    unsigned int m_flags;
};

} //io

} //infra
#endif // FILEPERMISSIONS_H
