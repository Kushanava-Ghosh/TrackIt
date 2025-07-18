#ifndef UTILS_MANAGER_H
#define UTILS_MANAGER_H

#include <string>
using namespace std;

class UtilsManager
{
    public:
        static string getCurrentBranch();
        static string hashFile(string fileContent);
        static string getLatestSubmit(string path);
        static string getTimestamp();
        static string parseDateTime(string dateTime);
        static void writeObject(string hash, string fileContent);
        static void deleteObject(string hash);
};

#endif