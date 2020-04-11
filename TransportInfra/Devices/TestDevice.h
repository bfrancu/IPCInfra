#ifndef TESTDEVICE_H
#define TESTDEVICE_H

#include "Observable.hpp"
#include "FileIODefinitions.h"

namespace infra
{

class TestDevice
{
    template<typename, typename, typename>
    friend class ErrorChangeAdvertiserPolicy;

public:
    TestDevice() :
        Error{io::EFileIOError::E_ERROR_UNKNOWN}
    {}

    TestDevice(io::EFileIOError err) :
        Error(err)
    {}

public:
    //const Observable<io::EFileIOError> & getError() const {return Error;}
    void setError(io::EFileIOError err) {Error = err;}
    int getHandle() const {return -1;}

protected:
    friend class ErrorMemberAccess;
    friend class AEIOUMemberAccess;

    const Observable<io::EFileIOError> & getError(){
        return Error;
    }

    const Observable<int> getAEIOU(){
        return AEIOU;
    }

public:
    Observable<int> AEIOU {0};

private:
    Observable<io::EFileIOError> Error;
};


}

#endif // TESTDEVICE_H
