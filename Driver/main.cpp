#include <iostream>
#include "FilePermissions.h"

using namespace std;

int main()
{
    using namespace infra::io;
    FilePermissions perm;
    perm.setFlags(1);
    cout << "Hello World!" << endl;
    return 0;
}
