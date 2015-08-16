/*
    Initial author: (https://github.com/)Convery
    License: LGPL 3.0
    Started: 2015-08-16
    Notes:
        Services handles packets synchronously based on internal criteria.
        Each server specifies these criteria in their own module.
*/

#pragma once
#include <stdint.h>
#include <string>

class IService
{
protected:
    // Internal storage to be used by the handler.
    uint8_t LocalStorage[16];
    
    // The data can be accessed from servers only.
    friend class IServer;

public:
    // Get service info and handler.
    virtual uint32_t GetServiceID() = 0;
    virtual bool HandlePacket(class IServer *Caller, std::basic_string<uint8_t> &PacketData) = 0;
};
