#ifndef OPERATION_MANAGER_H
#define OPERATION_MANAGER_H

#include <string>
#include <vector>
using namespace std;

class OperationManager
{
    private:
        string hashFile(string fileContent);
        void writeObject(string hash, string fileContent);
        void deleteObject(string hash);
};

#endif