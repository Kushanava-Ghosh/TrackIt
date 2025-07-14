#include "OperationManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iomanip>
#include <openssl/sha.h>
#include <map>

namespace fs = std::filesystem;

void OperationManager::store(vector<string> path)
{
    int n = path.size();
    for(int i = 0; i<n; i++)
    {
        if(!fs::exists(path[i]))
        {
            cerr << "File not found : " << path[i] << endl;
            cout << "Aborting staging operation !!!" << endl;
            return;
        }
    }

    ifstream indexRead(".trackit/index");
    string line;
    map <string, string> indexMap;
    map <string, int> objectsMap;
    while(getline(indexRead, line))
    {
        string file, hash;
        istringstream stream(line);
        stream >> file >> hash;
        indexMap[file] = hash;
        objectsMap[hash]++;
    }
    indexRead.close();

    auto stageFile = [&] (string path)
    {
        ifstream file(path);
        ostringstream stream;
        stream << file.rdbuf();
        string content = stream.str();
        string hash = hashFile(content);
        
        writeObject(hash, content);

        if(indexMap.find(path) != indexMap.end())
        {
            if(indexMap[path] != hash)
            {
                objectsMap[indexMap[path]]--;
                if(!objectsMap[indexMap[path]])
                deleteObject(indexMap[path]);
            }
        }

        objectsMap[hash]++;
        indexMap[path] = hash;
    };

    for(int i = 0; i<n; i++)
    {
        if(fs::is_regular_file(path[i]))
        stageFile(path[i]);
        else if(fs::is_directory(path[i]))
        {
            for(auto& entry : fs::recursive_directory_iterator(path[i]))
            if(fs::is_regular_file(entry.path()))
            stageFile(entry.path().generic_string());
        }
    }

    ofstream indexWrite(".trackit/index");
    for(auto [file, hash] : indexMap)
    indexWrite << file << " " << hash << endl;
    indexWrite.close();
}

void OperationManager::restore(vector<string> path, bool undo)
{
    int n = path.size();
    map <string, string> indexMap;
    map <string, int> objectsMap;
    string line;
    ifstream indexRead(".trackit/index");
    while(getline(indexRead, line))
    {
        string file, hash;
        istringstream stream(line);
        stream >> file >> hash;
        indexMap[file] = hash;
        objectsMap[hash]++;
    }
    indexRead.close();

    for(int i = 0; i<n; i++)
    {
        if(fs::is_regular_file(path[i]))
        {
            if(indexMap.find(path[i]) == indexMap.end())
            {
                cerr << "File not stored / No such file known to trackit : " << path[i] << endl;
                cout << "Aborting restoring operation !!!" << endl;
                return;
            }
        }
        else if(fs::is_directory(path[i]))
        {
            bool present = false;
            for(auto& [file, _] : indexMap)
            {
                if(file.rfind(path[i] + "/", 0) == 0)
                {
                    present = true;
                    break;
                }
            }

            if(!present)
            {
                cerr << "No tracked files found inside directory : " << path[i] << endl;
                cout << "Aborting restoring operation !!!" << endl;
                return;
            }
        }
    }

    if(undo)
    {
        auto undoFile = [&] (string path)
        {
            string sourcePath = ".trackit/objects/" + indexMap[path];
            string destinationPath = path;
            ifstream srcFile(sourcePath);
            if(!srcFile)
            {
                cerr << "Object not found: " << sourcePath << endl;
                return;
            }
            ofstream destFile(destinationPath);
            if(!destFile)
            {
                cerr << "Unable to write to: " << destinationPath << endl;
                return;
            }

            destFile << srcFile.rdbuf();

            srcFile.close();
            destFile.close();
        };

        for(int i = 0; i<n; i++)
        {
            if(fs::is_regular_file(path[i]))
            undoFile(path[i]);
            else if(fs::is_directory(path[i]))
            {
                for(auto& entry : fs::recursive_directory_iterator(path[i]))
                {
                    if(fs::is_regular_file(entry.path()))
                    undoFile(entry.path().generic_string());
                }
            }   
        }
    }
    else
    {
        auto unStageFile = [&] (string path)
        {
            string hash = indexMap[path];
            objectsMap[hash]--;
            if(!objectsMap[hash])
            deleteObject(hash);
            indexMap.erase(path);
        };
    
        for(int i = 0; i<n; i++)
        {
            if(fs::is_regular_file(path[i]))
            unStageFile(path[i]);
            else if(fs::is_directory(path[i]))
            {
                for(auto& entry : fs::recursive_directory_iterator(path[i]))
                if(fs::is_regular_file(entry.path()))
                unStageFile(entry.path().generic_string());
            }
        }
    
        ofstream indexWrite(".trackit/index");
        for(auto [file, hash] : indexMap)
        indexWrite << file << " " << hash << endl;
        indexWrite.close();
    }
}

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