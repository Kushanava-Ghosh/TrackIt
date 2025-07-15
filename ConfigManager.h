#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <map>
#include <string>
using namespace std;

class ConfigManager 
{
    private:
        string configPath;
        map <string, string> configMap;
    
        void loadConfig();
        void saveConfig();
    
    public:
        ConfigManager();
        void setConfig(const string& key, const string& value);
        string getConfig(const string& key);
};

#endif