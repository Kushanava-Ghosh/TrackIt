#include "UtilsManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <openssl/sha.h>
#include <iomanip>

namespace fs = std::filesystem;

string UtilsManager::getCurrentBranch()
{
    ifstream headFile(".trackit/HEAD");
    string ref;
    getline(headFile, ref);
    headFile.close();

    return ref;
}

string UtilsManager::hashFile(string fileContent)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char*)fileContent.c_str(), fileContent.size(), hash);
    ostringstream stream;
    for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
    stream << hex << setw(2) << setfill('0') << (int)hash[i];
    return stream.str();
}

string UtilsManager::getLatestSubmit(string path)
{
    ifstream refFile(".trackit/" + path);
    string submitId;
    getline(refFile, submitId);
    refFile.close();

    return submitId;
}

string UtilsManager::getTimestamp()
{
    time_t now = time(nullptr);
    tm local = *localtime(&now);
    tm utc = *gmtime(&now);

    int offset = static_cast<int>(mktime(&local) - mktime(&utc));
    int hours = offset / 3600;
    int minutes = abs((offset % 3600) / 60);

    ostringstream stream;
    stream << put_time(&local, "%d-%m-%YT%H:%M:%S") << " ";
    stream << (hours >= 0 ? "+" : "-") << setw(2) << setfill('0') << abs(hours) << ":" << setw(2) << setfill('0') << minutes;

    return stream.str();
}

string UtilsManager::parseDateTime(string dateTime)
{
    tm tm{};
    istringstream stream(dateTime);
    stream >> get_time(&tm, "%d-%m-%YT%H:%M:%S");
    time_t time = mktime(&tm);
    char parsedTime[100];
    strftime(parsedTime, sizeof(parsedTime), "%a %b %d %H:%M:%S %Y", localtime(&time));
    return string(parsedTime);
}

void UtilsManager::writeObject(string hash, string fileContent)
{
    if(!fs::exists(".trackit/objects/" + hash))
    {
        ofstream file(".trackit/objects/" + hash);
        file << fileContent;
        file.close();
    }
}

void UtilsManager::deleteObject(string hash)
{
    if(fs::exists(".trackit/objects/" + hash))
    fs::remove(".trackit/objects/" + hash);
}