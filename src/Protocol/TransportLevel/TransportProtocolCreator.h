#pragma once

#include "stdint.h"
#include "Protocol/Common.h"
#include "Protocol/CommonLibEmPulse/MemPack.h"

class TransportProtocolCreator
{
private:
    enum constants
    {
        MAX_SIZE_PACKET = 260,
        FLAGS_START_OFFSET = 0x1,
    };

    struct PacketData
    {
        PacketData *next;
        uint8_t    *data;
        TransportProtocolHeader header;
    };


private:
    uint32_t     m_flags           = 0;
    uint16_t     m_frame_num       = 0;

    uint32_t     m_data_max_length = MAX_SIZE_PACKET - sizeof(TransportProtocolHeader);
    uint32_t     m_data_offset     = 0;

    PacketData *m_list      = nullptr;
    PacketData *m_list_free = nullptr;
    PacketData *m_pos       = nullptr;

    ToolAddress m_address_source;
    ToolAddress m_address_destination;


public:
    TransportProtocolCreator();
    ~TransportProtocolCreator();

    void     SetAddress(const ToolAddress *address_source, const ToolAddress *address_destination);

    bool     Init();
    bool     Stop();
    bool     AddBody(const uint8_t *data, uint32_t size);
    
    uint8_t *PacketFirst(uint32_t *size);
    uint8_t *PacketNext(uint32_t *size);


private:
    bool        ListInit(PacketData *item);
    PacketData *ListCreate();
    PacketData *ListGet();
    void        ListRelease(PacketData *head);

    uint8_t    *ListDataGet(uint32_t *size, const PacketData *item) const;
};