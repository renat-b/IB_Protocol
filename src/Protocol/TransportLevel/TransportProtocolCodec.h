#pragma once

#include "stdint.h"
#include "Protocol/Common.h"

typedef bool(*TransportProtocolCoderNotify) (void *param, const uint8_t *buffer, uint32_t size);

class TransportProtocolCodec
{
private:
    enum constants
    {
        MAX_BUFFER = TRANSPORT_MAX_SIZE_PACKET,

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
    uint16_t    m_frame_num    = 0;
    uint32_t    m_last_error   = LAST_ERROR_SUCCESS;

    ToolAddress m_address_source;
    ToolAddress m_address_destination;
   
    uint8_t     m_buffer[MAX_BUFFER];
    uint32_t    m_buffer_max = TRANSPORT_MAX_SIZE_PACKET;
    uint32_t    m_data_offset = 0;
    
    TransportProtocolCoderNotify m_notify = NULL;
    void  *m_notify_param = NULL;


public:
    TransportProtocolCodec();
    ~TransportProtocolCodec();

    void        SetAddress(const ToolAddress *address_source, const ToolAddress *address_destination);
    void        SetNotify(void *param, TransportProtocolCoderNotify notify);
    uint32_t    GetLastError() const;

    bool        Initialize();
    bool        Create(const uint8_t *data, uint32_t size);
    bool        CreateEnd();


private:
    PacketData  *PacketGet();
    bool         PacketNotify(PacketData *packet, uint32_t flags);

    uint8_t     *PacketGetData(const PacketData *packet);
    uint32_t     PacketSizeMax();
    uint32_t     PacketSizeFree(const PacketData *packet);
};