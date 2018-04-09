#pragma once

#include "stdint.h"
#include "Protocol/Common.h"
#include "Protocol/CommonLibEmPulse/MemPack.h"

class TransportLevelCreator
{
private:
    enum constants
    {
        STATE_BEGIN = 1,
        STATE_CREATOR = 2,

        MAX_SIZE  = 260,
        TRANSPORT_LEVEL_VERSION = 1,
    };

private:
    uint32_t     m_state = STATE_BEGIN;
    MemPack      m_buffer;
    IndigoBaseTransportHeader m_header;
    uint16_t     m_frame_num = 0;

public:
    TransportLevelCreator();
    ~TransportLevelCreator();
    
    bool    AddData(const uint8_t *data, uint32_t size);

private:
    bool    Initialize();
    void    Clear();
};