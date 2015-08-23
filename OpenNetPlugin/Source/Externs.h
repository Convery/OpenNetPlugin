/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-20
    Notes:
        Global variables.
*/

#pragma once
#include <vector>
#include <unordered_map>
#include "Utility\lConsole.h"

namespace Global
{
    // Author information.
    static const char *PluginName = "Opennet";
    static const char *AuthorName = "Convery";    
    static const char *AuthorSite = "https://github.com/Convery";

    // Environmental information.
    extern std::vector<const char *> Dependencies;
    extern std::unordered_map<const char */*Key*/, const char */*Value*/> EnvKeyVal;
    extern lConsole PluginConsole;

    // Plugin communication.
    extern std::unordered_map < const char */*Statement*/, void */*Callback*/> ComCallbacks;
}
