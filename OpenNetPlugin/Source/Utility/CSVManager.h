/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-24
    Notes:
        Reads/writes a CSV file into/from a buffer.
*/

#pragma once
#include "..\STDInclude.h"

class CSVManager
{
public:
    std::vector<std::vector<std::string>> *Buffer{ nullptr };

    uint32_t ReadFile(const char *Filename, uint32_t MaxElementCount);
    uint32_t WriteFile(const char *Filename, uint32_t MaxElementCount);
};
