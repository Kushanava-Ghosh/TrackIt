#include "ConfigManager.h"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>

ConfigManager::ConfigManager()
{
    string root = getenv("USERPROFILE");
    configPath = root + "./.trackitconfig";
    loadConfig();
}

void ConfigManager::loadConfig()
{
    ifstream file(configPath);
    if(!file.is_open())
    return;
    string line, section;
    while(getline(file, line))
    {
        line.erase(remove_if(line.begin(), line.end(), [](unsigned char c) { return isspace(c); }), line.end());
        if(line.empty()) 
        continue;
        if(line.front() == '[' && line.back() == ']')
        section = line.substr(1, line.size() - 2);
        else if(line.find('=') != string::npos)
        {
            string key, value;
            auto position = line.find('=');
            key = line.substr(0, position);
            value = line.substr(position + 1);
            configMap[section + "." + key] = value;
        }
    }
    file.close();
}

void ConfigManager::saveConfig()
{
    ofstream file(configPath);
    if(!file.is_open())
    return;
    map<string, vector<pair<string, string>>> sectionMap;
    for(auto& [mainKey, value] : configMap)
    {
        string section, key;
        auto position = mainKey.find(".");
        if(position == string::npos)
        continue;
        section = mainKey.substr(0, position);
        key = mainKey.substr(position + 1);
        sectionMap[section].emplace_back(key, value);
    }
    for(auto& [section, entries] : sectionMap)
    {
        file << "[" << section << "]\n";
        for(auto& [key, value] : entries)
        file << "\t" << key << " = " << value << "\n";
    }
    file.close();
}

void ConfigManager::setConfig(const string& key, const string& value)
{
    configMap[key] = value;
    saveConfig();
}

string ConfigManager::getConfig(const string& key)
{
    return configMap.count(key) ? configMap[key] : ""; 
}