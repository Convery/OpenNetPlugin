/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-16
    Notes:
        Reduced this file to only include what's used.
*/

#pragma once
#include <stdint.h>

// Fast, not secure hashing.
inline uint32_t FNV1_32Hash(void *Data, uint32_t Length)
{
    uint32_t Prime = 16777619u;
    uint32_t Offset = 2166136261u;

    for (uint32_t i = 0; i < Length; i++)
        Offset = (Offset * Prime) ^ ((uint8_t *)Data)[i];

    return Offset;
}
inline uint64_t FNV1_64Hash(void *Data, uint32_t Length)
{
    uint64_t Prime = 1099511628211u;
    uint64_t Offset = 14695981039346656037u;

    for (uint32_t i = 0; i < Length; i++)
        Offset = (Offset * Prime) ^ ((uint8_t *)Data)[i];

    return Offset;
}
