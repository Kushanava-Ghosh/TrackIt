#ifndef SETUP_MANAGER_H
#define SETUP_MANAGER_H

#include <string>
using namespace std;

class SetupManager
{
    private:
        void createDirectory();
        void makeFile(string path);
        void setupMain();
    public:
        void setup();
};

#endif