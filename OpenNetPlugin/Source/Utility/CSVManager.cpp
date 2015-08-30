/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-24
    Notes:
        Reads/writes a CSV file into/from a buffer.
*/

#include "CSVManager.h"

uint32_t CSVManager::ReadFile(const char *Filename, uint32_t MaxElementCount)
{
    FILE *InputFile = fopen(Filename, "r");
    char InputString[1024];
    std::vector<std::string> Tokens;
    char *TempString;

    // Allocate a new buffer for the results.
    if (Buffer != nullptr)
    {
        Buffer->clear();
        delete Buffer;
    }
    Buffer = new std::vector<std::vector<std::string>>();

    // If the file is not found, return null.
    if (InputFile == NULL)
        return 0;

    // For each line in the input file.
    while (fgets(InputString, 1024, InputFile))
    {
        // If the line is empty or a comment, skip.
        if (strlen(InputString) < 2 || InputString[0] == '#')
            continue;

        // Split the string.
        Tokens.push_back(strtok(InputString, ","));
        while (true)
        {
            TempString = strtok(NULL, ",");

            // If a token is found.
            if (TempString != nullptr)
                Tokens.push_back(TempString);
            else
                break;

            // Remove the newline at the end of the line.
            if (Tokens.back().back() == '\n')
                Tokens.back().pop_back();
        }

        // Remove extra results.
        if (Tokens.size() > MaxElementCount)
            Tokens.resize(MaxElementCount);

        // Set the result and clear the buffers.
        Buffer->push_back(Tokens);
        Tokens.clear();
        memset(InputString, 0, 1024);
    }

    // Return the linecount.
    fclose(InputFile);
    return (uint32_t)Buffer->size();
}
uint32_t CSVManager::WriteFile(const char *Filename, uint32_t MaxElementCount)
{
    DeleteFileA(Filename);
    FILE *InputFile = fopen(Filename, "a");

    // If the file or buffer is not found, return null.
    if (InputFile == NULL || Buffer == nullptr)
        return 0;

    // Print a fileheader.
    fprintf(InputFile, "# This file is generated automatically.\n");
    fprintf(InputFile, "# Please do not modify unless you know what you're doing.\n");
    fprintf(InputFile, "# Contact the author (%s) for help.\n\n", Global::AuthorName);

    // For each line in the buffer.
    for (uint32_t i = 0; i < Buffer->size(); i++)
    {
        // For each token on the line.
        for (uint32_t c = 0; c < min(MaxElementCount, (*Buffer)[i].size()); c++)
        {
            fprintf(InputFile, "%s%s", (*Buffer)[i][c].c_str(), c == min(MaxElementCount, (*Buffer)[i].size()) - 1 ? "\n" : ",");
        }
    }

    // Return the linecount.
    fclose(InputFile);
    return (uint32_t)Buffer->size();
}
