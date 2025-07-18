#ifndef OPERATION_MANAGER_H
#define OPERATION_MANAGER_H

#include <string>
#include <vector>
using namespace std;

class OperationManager
{
    private:
        void setLatestSubmit(string path, string hash);
    public:
        void store(vector<string> path);
        void restore(vector<string> path, bool undo);
        void submit(string message, bool amend);
        void status();
        void log();
};

#endif