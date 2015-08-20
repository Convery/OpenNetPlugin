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

namespace Global
{
    // Author information.
    static const char *PluginName = "Opennet";
    static const char *AuthorName = "Convery";    
    static const char *AuthorSite = "https://github.com/Convery";

    // Environmental information.
    static std::vector<const char *> Dependencies = {};
    static std::unordered_map<const char */*Key*/, const char */*Value*/> EnvKeyVal;
    static void *StreamHandle;

    // Plugin communication.
    static std::unordered_map < const char */*Statement*/, void */*Callback*/> ComCallbacks;
}
