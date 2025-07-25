#include "BranchManager.h"
#include "ConfigManager.h"
#include "UtilsManager.h"
#define byte WindowsByte
#include "termcolor/termcolor.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <map>

namespace fs = std::filesystem;

void BranchManager::shift(string name, bool branch)
{
    ConfigManager cfg;
    
    if(branch)
    {
        string currentBranch = UtilsManager::getCurrentBranch();
        string latestSubmit = UtilsManager::getLatestSubmit(currentBranch);

        if(fs::exists(".trackit/refs/heads/" + name))
        {
            cerr << termcolor::bright_red << "Fatal: Branch already exists -> " << termcolor::yellow << name << endl << termcolor::reset;
            return;
        }

        cout << termcolor::green << "Shifted to a new branch -> " << termcolor::yellow << name << endl << termcolor::reset;

        ofstream refFile(".trackit/refs/heads/" + name);
        refFile << latestSubmit;
        refFile.close();

        ofstream logFile(".trackit/logs/refs/heads/" + name);
        logFile << string(40, '0') << " " << latestSubmit << " " << cfg.getConfig("user.name") << " [" << cfg.getConfig("user.email") << "] " << UtilsManager::getTimestamp() << " branch: Created from " << currentBranch << endl;
        logFile.close();
        
        ofstream headFile(".trackit/HEAD");
        headFile << "refs/heads/" + name;
        headFile.close();
    }
    else
    {
        if(!fs::exists(".trackit/refs/heads/" + name))
        {
            cerr << termcolor::bright_red << "Fatal: Branch does not exists -> " << termcolor::yellow << name << endl << termcolor::reset;
            return;
        }
        string prevBranch = UtilsManager::getCurrentBranch();
        string prevSubmit = UtilsManager::getLatestSubmit(prevBranch);

        if(prevBranch == "refs/heads/" + name)
        {
            cerr << termcolor::bright_red << "Error: Already on Branch -> " << termcolor::yellow << name << endl << termcolor::reset;
            return;
        }

        ifstream lastFile(".trackit/objects/" + prevSubmit);
        string line;
        while(getline(lastFile, line))
        {
            string file, hash;
            istringstream stream(line);
            stream >> file >> hash;
            fs::path dir = fs::path(file).parent_path();
            fs::remove(file);
            while(fs::exists(dir) && dir != fs::current_path() && fs::is_empty(dir))
            {
                fs::remove(dir);
                dir = dir.parent_path();
            }
        }
        lastFile.close();

        ofstream headFile(".trackit/HEAD");
        headFile << "refs/heads/" + name;
        headFile.close();

        string currentBranch = UtilsManager::getCurrentBranch();
        string latestSubmit = UtilsManager::getLatestSubmit(currentBranch);

        ifstream submitFile(".trackit/objects/" + latestSubmit);
        while(getline(submitFile, line))
        {
            string file, hash;
            istringstream stream(line);
            stream >> file >> hash;
            ifstream blob(".trackit/objects/" + hash);
            fs::path filePath(file);
            if(!filePath.parent_path().empty() && !fs::exists(filePath.parent_path()))
            fs::create_directories(filePath.parent_path());
            ofstream File(file);
            File << blob.rdbuf();
            File.close();
            blob.close();
        }
    }
}

void BranchManager::del(string name)
{
    if(!fs::exists(".trackit/refs/heads/" + name))
    {
        cerr << termcolor::bright_red << "Fatal: Branch -> " << termcolor::yellow << name << termcolor::bright_red << " not found" << endl << termcolor::reset;
        return;
    }

    if(!UtilsManager::getCurrentBranch().compare("refs/heads/" + name))
    {
        cerr << termcolor::bright_red << "Fatal: Cannot delete branch -> " << termcolor::yellow << name << termcolor::bright_red << " used by worktree" << endl << termcolor::reset;
        return;
    }

    string lastSubmit = UtilsManager::getLatestSubmit("refs/heads/" + name);

    fs::remove(".trackit/refs/heads/" + name);
    fs::remove(".trackit/logs/refs/heads/" + name);
    cout << termcolor::bright_cyan << "Deleted Branch -> " << termcolor::yellow << name << termcolor::bright_blue << " [was " << lastSubmit << "]" << endl << termcolor::reset;
}

void BranchManager::branch()
{
    string currentBranch = UtilsManager::getCurrentBranch();
    string name = currentBranch.substr(currentBranch.find_last_of('/') + 1);

    string path = ".trackit/refs/heads/";
    for(auto& entry : fs::directory_iterator(path))
    {
        if(!name.compare(entry.path().filename().string()))
        cout << termcolor::yellow << "--> " << termcolor::green << name << termcolor::reset << endl;
        else
        cout << "    " << entry.path().filename().string() << endl;
    }
}

