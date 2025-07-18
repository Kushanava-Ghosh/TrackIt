#include "OperationManager.h"
#include "ConfigManager.h"
#include "UtilsManager.h"
#define byte WindowsByte
#include "termcolor/termcolor.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iomanip>
#include <map>

namespace fs = std::filesystem;

struct LogInfo {
    string hash;
    string authorName;
    string authorEmail;
    string datetime;
    string timezone;
    string label;
    string message;
};

void OperationManager::store(vector<string> path)
{
    int n = path.size();
    for(int i = 0; i<n; i++)
    {
        if(!fs::exists(path[i]))
        {
            cout << termcolor::bright_red;
            cerr << "File not found : " << path[i] << endl;
            cout << "Aborting staging operation !!!" << endl;
            cout << termcolor::reset;
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

    string currentBranch = UtilsManager::getCurrentBranch();
    string latestSubmit = UtilsManager::UtilsManager::getLatestSubmit(currentBranch);

    ifstream submitFile(".trackit/objects/" + latestSubmit);
    map <string, string> snapshot;
    while(getline(submitFile, line))
    {
        string file, hash;
        istringstream stream(line);
        stream >> file >> hash;
        snapshot[file] = hash;
    }
    submitFile.close();

    auto stageFile = [&] (string path)
    {
        ifstream file(path);
        ostringstream stream;
        stream << file.rdbuf();
        string content = stream.str();
        string hash = UtilsManager::hashFile(content);
        
        if(snapshot[path] != hash)
        {
            UtilsManager::writeObject(hash, content);
    
            if(indexMap.find(path) != indexMap.end())
            {
                if(indexMap[path] != hash)
                {
                    objectsMap[indexMap[path]]--;
                    if(!objectsMap[indexMap[path]])
                    UtilsManager::deleteObject(indexMap[path]);
                }
            }
    
            objectsMap[hash]++;
            indexMap[path] = hash;
        }
        else
        indexMap.erase(path);
        file.close();
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

    string currentBranch = UtilsManager::getCurrentBranch();
    string latestSubmit = UtilsManager::getLatestSubmit(currentBranch);

    ifstream submitFile(".trackit/objects/" + latestSubmit);
    map <string, string> snapshot;
    while(getline(submitFile, line))
    {
        string file, hash;
        istringstream stream(line);
        stream >> file >> hash;
        snapshot[file] = hash;
    }
    submitFile.close();

    for(int i = 0; i<n; i++)
    {
        if(fs::is_regular_file(path[i]))
        {
            if(indexMap.find(path[i]) == indexMap.end() && snapshot.find(path[i]) == snapshot.end())
            {
                cout << termcolor::bright_red;
                cerr << "File not stored / No such file known to trackit : " << path[i] << endl;
                cout << "Aborting restoring operation !!!" << endl;
                cout << termcolor::reset;
                return;
            }
        }
        else if(fs::is_directory(path[i]))
        {
            bool presentIndex = false, presentSnapshot = false;
            for(auto& [file, _] : indexMap)
            {
                if(file.rfind(path[i] + "/", 0) == 0)
                {
                    presentIndex = true;
                    break;
                }
            }

            for(auto& [file, _] : snapshot)
            {
                if(file.rfind(path[i] + "/", 0) == 0)
                {
                    presentSnapshot = true;
                    break;
                }
            }
            if(!(presentIndex || presentSnapshot))
            {
                cout << termcolor::bright_red;
                cerr << "No tracked files found inside directory : " << path[i] << endl;
                cout << "Aborting restoring operation !!!" << endl;
                cout << termcolor::reset;
                return;
            }
        }
    }

    if(undo)
    {
        auto undoFile = [&] (string path)
        {
            string sourcePath = ".trackit/objects/";
            if(indexMap.find(path) != indexMap.end())
            sourcePath += indexMap[path];
            else if(snapshot.find(path) != snapshot.end())
            sourcePath += snapshot[path];
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
            UtilsManager::deleteObject(hash);
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

void OperationManager::submit(string message, bool amend)
{
    if(!message.size() && !amend)
    {
        cout << termcolor::bright_red;
        cerr << "Aborting Submit due to Empty Submit Message" << endl;
        cout << termcolor::reset;
        return;
    }

    ifstream indexRead(".trackit/index");
    map <string, string> snapshot;
    string line;
    while(getline(indexRead, line))
    {
        string file, hash;
        istringstream stream(line);
        stream >> file >> hash;
        snapshot[file] = hash;
    }
    indexRead.close();

    if(!snapshot.size())
    {
        if(!amend)
        {
            cout << termcolor::bright_red;
            cerr << "Staging Area Empty or Missing." << endl;
            cout << "Aborting Submit operation !!!";
            cout << termcolor::reset;
            return;
        }
        else if(!message.size())
        {
            cerr << termcolor::bright_red << "Nothing to amend !!!" << endl;
            cout << termcolor::yellow << "  (use \"trackit submit --amend -m <message>\" to change submit message and update the stored changes into previous submit)" << endl;
            cout << termcolor::bright_red << "Aborting Submit operation !!!" << termcolor::reset << endl;
            return;
        }
    }

    string currentBranch = UtilsManager::getCurrentBranch();
    string latestSubmit = UtilsManager::getLatestSubmit(currentBranch);

    if(latestSubmit.compare(string(40, '0')))
    {
        ifstream submitFile(".trackit/objects/" + latestSubmit);
        while(getline(submitFile, line))
        {
            string file, hash;
            istringstream stream(line);
            stream >> file >> hash;
            if(snapshot.find(file) == snapshot.end())
            snapshot[file] = hash;
        }
        submitFile.close();
    }

    ofstream indexWrite(".trackit/index");
    for(auto [file, hash] : snapshot)
    indexWrite << file << " " << hash << endl;
    indexWrite.close();

    ifstream indexReadFile(".trackit/index");
    ostringstream stream;
    stream << indexReadFile.rdbuf();
    string submitContent = stream.str();
    string submitHash = UtilsManager::hashFile(submitContent);

    UtilsManager::writeObject(submitHash, submitContent);
    
    ofstream clearFile(".trackit/index", ios::trunc);
    clearFile.close();

    setLatestSubmit(currentBranch, submitHash);
    
    ConfigManager cfg;
    string author = cfg.getConfig("user.name");
    string email = cfg.getConfig("user.email");
    string time = UtilsManager::getTimestamp();

    ofstream localLog(".trackit/logs/" + currentBranch, ios::app);
    localLog << latestSubmit << " " << submitHash << " " << author << " [" << email << "] " << time << " " << "submit";
    if(amend)
    {
        if(latestSubmit != submitHash)
        UtilsManager::deleteObject(latestSubmit);
        if(!message.size())
        {
            ifstream logFile(".trackit/logs/" + currentBranch);
            string prevLine, line;
            while(getline(logFile, line))
            {
                if(!line.empty())
                prevLine = line;
            }
            logFile.close();

            string prevHash, newHash, author, email, dateTime, timeZone, label, oldMessage;
            istringstream stream(prevLine);
            stream >> prevHash >> newHash >> author >> email >> dateTime >> timeZone >> label;
            getline(stream, oldMessage);
            
            message = oldMessage.substr(1);
        }
        localLog << "(amend)";
    }
    else
    {
        if(!latestSubmit.compare(string(40, '0')))
        localLog << "(initial)";
    }
    localLog << ": " << message << endl;
    localLog.close();
}

void OperationManager::status()
{
    fs::path curr_path = fs::current_path();
    vector<string> directory;
    bool diff = false;
    for(auto& entry : fs::directory_iterator(curr_path))
    {
        if (entry.path().filename() == ".trackit") 
        continue;
        directory.push_back(entry.path().filename().generic_string());
    }

    string currentBranch = UtilsManager::getCurrentBranch();
    string latestSubmit = UtilsManager::getLatestSubmit(currentBranch);
    string prefix = "refs/heads/";
    cout << "On branch " << currentBranch.substr(prefix.length()) << endl;
    map <string, string> indexMap, snapshot;
    string line;

    ifstream indexRead(".trackit/index");
    while(getline(indexRead, line))
    {
        string file, hash;
        istringstream stream(line);
        stream >> file >> hash;
        indexMap[file] = hash;
    }
    indexRead.close();

    ifstream submitRead(".trackit/objects/" + latestSubmit);
    while(getline(submitRead, line))
    {
        string file, hash;
        istringstream stream(line);
        stream >> file >> hash;
        snapshot[file] = hash;
    }
    submitRead.close();

    map <string, string> liveSnapshot;
    auto readFile = [&] (string path)
    {
        ifstream file(path);
        ostringstream stream;
        stream << file.rdbuf();
        string content = stream.str();
        string hash = UtilsManager::hashFile(content);
        liveSnapshot[path] = hash;
        if(!diff)
        {
            if(indexMap.find(path) != indexMap.end())
            {
                if(indexMap[path] != hash)
                diff = true;
                else
                diff = false;
            }
            else if(snapshot.find(path) != snapshot.end() && snapshot[path] != hash)
            diff = true;
        }
    };

    for(auto item : directory)
    {
        if(fs::is_regular_file(item))
        readFile(item);
        else if(fs::is_directory(item))
        {
            for(auto& entry : fs::recursive_directory_iterator(item))
            {
                if(fs::is_regular_file(entry.path()))
                readFile(entry.path().generic_string());
            }
        }
    }

    if(indexMap.size())
    {
        cout << "Changes to be submitted:" << endl;
        cout << termcolor::cyan << "  (use \"trackit restore <file>...\" to unstage files)" << endl;
        cout << termcolor::green;
        for(auto [file, _] : indexMap)
        {
            if(indexMap[file] == liveSnapshot[file])
            liveSnapshot.erase(file);
            if(snapshot.find(file) != snapshot.end())
            cout << "  \tmodified:    ";
            else
            cout << "  \tnew file:    ";
            cout << file << endl;
        }
        cout << termcolor::reset;
        cout << endl;
    }
    
    if(diff)
    {
        cout << "Changes not stored for submit:" << endl;
        cout << termcolor::cyan;
        cout << "  (use \"trackit store <file>...\" to update what will be submitted)" << endl;
        cout << "  (use \"trackit restore --undo <file>...\" to discard changes in working directory)" << endl;
        cout << termcolor::yellow;
        for(auto [file, hash] : liveSnapshot)
        {
            if(indexMap.find(file) != indexMap.end() && indexMap[file] != hash)
            cout << "  \tmodified:    " << file << endl;
            else if(snapshot.find(file) != snapshot.end() && snapshot[file] != hash)
            cout << "  \tmodified:    " << file << endl;
        }
        cout << termcolor::reset;
        cout << endl;
    }
    
    for(auto [file, _] : snapshot)
    {
        if(liveSnapshot.find(file) != liveSnapshot.end())
        liveSnapshot.erase(file);
    }
    
    if(liveSnapshot.size())
    {
        cout << "Untracked files:" << endl;
        cout << termcolor::cyan << "  (use \"trackit store <file>...\" to include in what will be submitted)" << endl;
        cout << termcolor::red;
        for(auto [file, _] : liveSnapshot)
        cout << "  \t" << file << endl;
        cout << termcolor::reset;
        cout << endl;
    }
}

void OperationManager::log()
{
    string currentBranch = UtilsManager::getCurrentBranch();
    vector<LogInfo> logs;
    string line;
    bool parent;
    string parentHash = "", currentHash = "";
    do
    {
        parent = false;
        ifstream file(".trackit/logs/" + currentBranch);
        while(getline(file, line))
        {
            string oldHash, newHash, author, email, dateTime, timeZone, label, message; 
            istringstream stream(line);
            stream >> oldHash >> newHash >> author >> email >> dateTime >> timeZone >> label;
            getline(stream, message);
            LogInfo entry {
                newHash,
                author,
                email,
                dateTime,
                timeZone,
                label,
                message.substr(1)
            };
            if(!label.compare("branch:"))
            {
                parent = true;
                string prefix = "Created from ";
                currentBranch = entry.message.substr(prefix.length());
                parentHash = newHash;
                continue;
            }
            if(!oldHash.compare(currentHash) && newHash.compare(currentHash))
            break;
            logs.push_back(entry);
        }
        file.close();
    
        bool skipLog = false;
        for(auto it = logs.rbegin(); it != logs.rend(); it++)
        {
            if(skipLog)
            {
                if(!((it->label).compare("submit(initial):")) || !((it->label).compare("submit:")))
                skipLog = false;
                continue;
            }
            if(!((it->label).compare("submit(amend):")))
            skipLog = true;
            cout << termcolor::green << "submit " << it->hash << termcolor::reset << endl;
            cout << termcolor::cyan << "Author: " << it->authorName << " " << it->authorEmail << termcolor::reset << endl;
            cout << termcolor::blue << "Date: " << UtilsManager::parseDateTime(it->datetime) << " " << it->timezone << termcolor::reset << endl;
            cout << endl << termcolor::magenta << "Message: " << termcolor::yellow << it->message << termcolor::reset << endl << endl;
        }
        logs.clear();
        currentHash = parentHash;
    } 
    while(parent);
}

void OperationManager::setLatestSubmit(string path, string hash)
{
    ofstream refFile(".trackit/" + path);
    refFile << hash;
    refFile.close();
}