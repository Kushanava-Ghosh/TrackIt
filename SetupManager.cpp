#include "SetupManager.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

void SetupManager::setup()
{
    fs::path root = fs::current_path() / ".trackit";
    string rootPath = root.string() + "/";
    replace(rootPath.begin(), rootPath.end(), '\\', '/');

    if(fs::exists(root))
    {
        cout << "Trackit Repository already exists in " << rootPath << endl;
        return;
    }
    createDirectory();

    setupMain();

    cout << "Empty Trackit Repository setup completed in " << rootPath << endl;
}

void SetupManager::createDirectory()
{
    fs::create_directories(".trackit/logs/refs/heads");
    fs::create_directories(".trackit/objects");
    fs::create_directories(".trackit/refs/heads");

    makeFile(".trackit/index");
    makeFile(".trackit/HEAD");
    makeFile(".trackit/logs/HEAD");
    makeFile(".trackit/logs/refs/heads/main");
    makeFile(".trackit/refs/heads/main");
}

void SetupManager::makeFile(string path)
{
    ofstream file(path);
    if(file.is_open())
    file.close();
    else
    cerr << "Failed to create file : " << path << endl;
}

void SetupManager::setupMain()
{
    ofstream headFile(".trackit/HEAD");
    headFile << ".trackit/refs/heads/main";
    headFile.close();

    ofstream refFile(".trackit/refs/heads/main");
    refFile << string(40, '0');
    refFile.close();
}