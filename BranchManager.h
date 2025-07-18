#ifndef BRANCH_MANAGER_H
#define BRANCH_MANAGER_H

#include <string>
using namespace std;

class BranchManager
{
    public:
        void shift(string name, bool branch);
        void del(string name);

};

#endif