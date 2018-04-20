#pragma once

#include "stdint.h"
#include "Protocol/Common.h"
#include "Protocol/CommonLibEmPulse/StreamBuffer.h"

typedef bool (*TransportProtocolDecodecNotify)(void *param, const TransportProtocolHeader *header, const uint8_t *body);


class TransportProtocolDecodec
{
private:
    enum constants
    {
        MAX_BUFFER   = 2 * TRANSPORT_MAX_SIZE_PACKET,
        STATE_HEADER = 1,
        STATE_BODY   = 2,

        LAST_ERROR_SUCCESS = 0,
        LAST_ERROR_NOT_ENOUGH_MEMORY,
        LAST_ERROR_NOT_ENOUGH_BUFFER,
        LAST_ERROR_CHECK_CRC_BODY,
        LAST_ERROR_CHECK_CRC_HEADER,
        LAST_ERROR_NULL_POINTER,
        LAST_ERROR_NOTIFY,
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
    uint32_t     m_buffer_max = MAX_BUFFER;
    
    uint32_t     m_state = STATE_HEADER;
    
    TransportProtocolHeader *m_header;

    TransportProtocolDecodecNotify m_notify = NULL;
    void  *m_notify_param = NULL;


public:
    TransportProtocolDecodec();
    ~TransportProtocolDecodec();

    bool     Initialize();
    void     SetNotify(void *param, TransportProtocolDecodecNotify notify);

    bool     Parse(const uint8_t *data, uint32_t size);
    uint32_t GetLastError();


private:
    bool     ValidateHeader(const TransportProtocolHeader *header);
    bool     ValidateBody(const TransportProtocolHeader *header, const uint8_t *body);

    bool     ParseHeader(const uint8_t **data, uint32_t *size);
    bool     ParseBody(const uint8_t **data, uint32_t *size);
};