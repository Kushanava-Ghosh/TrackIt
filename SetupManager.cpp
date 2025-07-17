#include "SetupManager.h"
#include "termcolor/termcolor.hpp"
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
        cout << termcolor::bright_red << "Trackit Repository already exists in " << termcolor::yellow << rootPath << endl << termcolor::reset;
        return;
    }
    createDirectory();

    setupMain();

    cout << termcolor::green << "Empty Trackit Repository setup completed in " << termcolor::yellow << rootPath << endl << termcolor::reset;
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
    cerr << termcolor::bright_red << "Failed to create file : " << termcolor::yellow << path << endl << termcolor::reset;
}

void SetupManager::setupMain()
{
    ofstream headFile(".trackit/HEAD");
    headFile << "refs/heads/main";
    headFile.close();

    ofstream refFile(".trackit/refs/heads/main");
    refFile << string(40, '0');
    refFile.close();
}