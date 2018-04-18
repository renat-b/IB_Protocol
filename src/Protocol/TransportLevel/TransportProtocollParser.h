#pragma once

#include "stdint.h"
#include "Protocol/Common.h"
#include "Protocol/CommonLibEmPulse/StreamBuffer.h"

class TransportProtocollParser
{
private:
    enum constants
    {
        MAX_BUFFER   = 24 * 1024,
        STATE_HEADER = 1,
        STATE_BODY   = 2,

        LAST_ERROR_SUCCESS = 0,
        LAST_ERROR_NOT_ENOUGH_MEMORY,
        LAST_ERROR_NOT_ENOUGH_BUFFER,

        LAST_ERROR_CHECK_CRC_BODY,
        LAST_ERROR_CHECK_CRC_HEADER,
        LAST_ERROR_NULL_POINTER,
    };

    struct PacketDataOld
    {
        PacketDataOld   *next;
        uint32_t      data_readed;
        uint8_t      *data;
        TransportProtocolHeader header;
    };

#pragma pack(push, 1)
    struct PacketData
    {
        TransportProtocolHeader header;
    };
#pragma pack(pop)


private:
    uint32_t     m_last_error = LAST_ERROR_SUCCESS;

    uint8_t      m_buffer[MAX_BUFFER];
    uint32_t     m_buffer_len = 0;
    uint32_t     m_buffer_pos = 0;


public:
    TransportProtocollParser();
    ~TransportProtocollParser();

    bool     Initialize();

    bool     AddData(const uint8_t *data, uint32_t size);
    bool     Parse();

    uint8_t *GetFirstData(uint32_t *size);
    uint8_t *GetNextData(uint32_t *size);

    uint32_t GetLastError();


private:
    bool     ValidateHeader(const TransportProtocolHeader *header);
    bool     ValidateBody(const TransportProtocolHeader *header, const uint8_t *body);
    uint8_t *GetData(uint32_t *size);
};