#include "OperationManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iomanip>
#include <openssl/sha.h>
#include <map>

namespace fs = std::filesystem;

string OperationManager::hashFile(string fileContent)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char*)fileContent.c_str(), fileContent.size(), hash);
    ostringstream stream;
    for(int i = 0; i < SHA_DIGEST_LENGTH; i++)
    stream << hex << setw(2) << setfill('0') << (int)hash[i];
    return stream.str();
}

void OperationManager::writeObject(string hash, string fileContent)
{
    if(!fs::exists(".trackit/objects/" + hash))
    {
        ofstream file(".trackit/objects/" + hash);
        file << fileContent;
        file.close();
    }
}

void OperationManager::deleteObject(string hash)
{
    if(fs::exists(".trackit/objects/" + hash))
    fs::remove(".trackit/objects/" + hash);
}