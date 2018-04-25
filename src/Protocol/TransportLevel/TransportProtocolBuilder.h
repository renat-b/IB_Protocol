#pragma once

#include "stdint.h"
#include "Protocol/Common.h"
#include "Protocol/TransportLevel/TransportProtocolFrameBuilder.h"

typedef bool(*TransportProtocolBuilderNotify) (const void *param, const uint8_t *buffer, uint32_t size);

class TransportProtocolBuilder
{
private:
    enum constants
    {
        MAX_BUFFER = TRANSPORT_MAX_SIZE_FRAME - sizeof(TransportProtocolHeader),

        LAST_ERROR_SUCCESS = 0,
        LAST_ERROR_NOT_ENOUGH_MEMORY,
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
    TransportProtocolFrameBuilder m_builder;
    uint32_t    m_last_error   = LAST_ERROR_SUCCESS;
    
    TransportProtocolBuilderNotify m_notify = NULL;
    const void  *m_notify_param = NULL;

    bool        m_first_frame = true;
    uint8_t     m_buffer_in[TRANSPORT_MAX_SIZE_BODY];
    uint32_t    m_buffer_in_pos;
    uint8_t     m_buffer_out[TRANSPORT_MAX_SIZE_FRAME];


public:
    TransportProtocolBuilder();
    ~TransportProtocolBuilder();

    uint32_t    GetLastError() const;

    void        SetAddress(const ToolAddress *address_source, const ToolAddress *address_destination);
    void        SetNotify(TransportProtocolBuilderNotify notify, const void *param);

    bool        Initialize();
    bool        BodyAdd(const uint8_t *body, uint32_t size);
    bool        BodyEnd();


private:
    bool        BuildFrame(const TransportProtocolHeader *header, const uint8_t *body);

private:
    static bool TransportProtocolCodeNotifyFunc(const void *param, const TransportProtocolHeader *header, const uint8_t *body);
};