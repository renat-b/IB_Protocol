#pragma once

#include "stdint.h"
#include "Protocol/Common.h"
#include "Protocol/CommonLibEmPulse/MemPack.h"

class TransportProtocolCreator
{
private:
    enum constants
    {
        MAX_BUFFER = 24 * 1024,

        LAST_ERROR_SUCCESS = 0,
        LAST_ERROR_NOT_ENOUGH_MEMORY = 1,
        LAST_ERROR_NULL_POINTER = 2,
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
   
    uint32_t    m_packet_num    = 0; 
    uint32_t    m_packet_pos    = 0;

    uint8_t     m_buffer[MAX_BUFFER];
    uint32_t    m_buffer_len    = 0;
    uint32_t    m_data_offset  = 0;


public:
    TransportProtocolCreator();
    ~TransportProtocolCreator();

    void        SetAddress(const ToolAddress *address_source, const ToolAddress *address_destination);
    uint32_t    GetLastError() const;

    bool        Initialize();
    bool        AddBody(const uint8_t *data, uint32_t size);
    bool        Build();
   
    uint8_t    *GetPacketFirst(uint32_t *size);
    uint8_t    *GetPacketNext(uint32_t *size);


private:
    PacketData  *PacketGet();
};