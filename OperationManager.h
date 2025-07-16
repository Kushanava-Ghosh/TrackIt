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
        string getCurrentBranch();
        string getLatestSubmit(string path);
        void setLatestSubmit(string path, string hash);
        string getTimestamp();
        string parseDateTime(string dateTime);
    public:
        void store(vector<string> path);
        void restore(vector<string> path, bool undo);
        void submit(string message, bool amend);
        void status();
        void log();
};

#endif