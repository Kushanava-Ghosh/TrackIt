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
        void setConfig(string& key, string& value);
        string getConfig(string& key);
};

#endif