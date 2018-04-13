#pragma once

#include "stdint.h"
#include "Protocol/Common.h"
#include "Protocol/CommonLibEmPulse/StreamBuffer.h"

class TransportProtocollParser
{
private:
    enum constants
    {
        STATE_HEADER = 1,
        STATE_BODY   = 2,
        MAX_SIZE     = 260,
    };

    struct PacketData
    {
        PacketData *next;
        uint32_t            data_readed;
        uint8_t            *data;
        TransportProtocolHeader header;
    };


private:
    uint32_t     m_state = STATE_HEADER;

    uint32_t     m_data_max_length = MAX_SIZE;
    uint32_t     m_data_offset = 0;

    PacketData  *m_list      = nullptr;
    PacketData  *m_list_free = nullptr;
    PacketData  *m_pos       = nullptr;


public:
    TransportProtocollParser();
    ~TransportProtocollParser();

    bool     Init();
    bool     ParseData(const uint8_t *data, uint32_t size);

    bool     GetFirstData(uint8_t *data, uint32_t *read_size);
    bool     GetNextData(uint8_t *data, uint32_t *read_size);

private:
    void     Shutdown();

    bool     ValidateHeader(const TransportProtocolHeader *header) const;
    bool     ValidateBody(const PacketData *data) const;
    PacketData   *ListGet();
    PacketData   *ListCreate();
};