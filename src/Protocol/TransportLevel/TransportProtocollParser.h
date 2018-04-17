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

        LAST_ERROR_SUCCESS = 0,
        LAST_ERROR_NOT_ENOUGH_MEMORY = 1,
        LAST_ERROR_CHECK_CRC_BODY = 2,
        LAST_ERROR_CHECK_CRC_HEADER = 3,
        LAST_ERROR_NULL_POINTER = 4,
    };

    struct PacketData
    {
        PacketData   *next;
        uint32_t      data_readed;
        uint8_t      *data;
        TransportProtocolHeader header;
    };


private:
    uint32_t     m_state = STATE_HEADER;
    uint32_t     m_last_error = LAST_ERROR_SUCCESS;

    uint32_t     m_data_max_length = TRANSPORT_MAX_SIZE_PACKET;

    PacketData  *m_list      = NULL;
    PacketData  *m_list_free = NULL;
    PacketData  *m_pos       = NULL;


public:
    TransportProtocollParser();
    ~TransportProtocollParser();

    bool     Initialize();
    bool     ParseData(const uint8_t *data, uint32_t size);

    bool     GetFirstData(uint8_t *data, uint32_t *read_size);
    bool     GetNextData(uint8_t *data, uint32_t *read_size);

    uint32_t GetLastError();

private:
    void     Shutdown();

    bool     ValidateHeader(const TransportProtocolHeader *header) const;
    bool     ValidateBody(const PacketData *data) const;
    PacketData   *ListGet();
    PacketData   *ListCreate();
    PacketData  **ListGetTail(PacketData **head);
};