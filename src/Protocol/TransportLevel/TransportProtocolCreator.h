#pragma once

#include "stdint.h"
#include "Protocol/Common.h"
#include "Protocol/CommonLibEmPulse/MemPack.h"

class TransportProtocolCreator
{
private:
    enum constants
    {
        FLAGS_START_OFFSET = 0x1,

        LAST_ERROR_SUCCESS = 0,
        LAST_ERROR_NOT_ENOUGH_MEMORY = 1,
        LAST_ERROR_NULL_POINTER = 2,
    };

#pragma pack(push, 1)
    struct PacketData
    {
        PacketData *next;
        uint8_t    *data;
        TransportProtocolHeader header;
    };
#pragma pack(pop)

private:
    uint32_t     m_flags           = 0;
    uint16_t     m_frame_num       = 0;
    uint32_t     m_last_error      = LAST_ERROR_SUCCESS;

    uint32_t     m_data_max_length = TRANSPORT_MAX_SIZE_PACKET - sizeof(TransportProtocolHeader);
    uint32_t     m_data_offset     = 0;

    PacketData *m_list      = NULL;
    PacketData *m_list_free = NULL;
    PacketData *m_pos       = NULL;

    ToolAddress m_address_source;
    ToolAddress m_address_destination;


public:
    TransportProtocolCreator();
    ~TransportProtocolCreator();

    void        SetAddress(const ToolAddress *address_source, const ToolAddress *address_destination);
    uint32_t    GetLastError() const;

    bool        Initialize();
    bool        MessageAddBody(const uint8_t *data, uint32_t size);
    bool        MessageStop();
   
    uint8_t    *GetPacketFirst(uint32_t *size);
    uint8_t    *GetPacketNext(uint32_t *size);


private:
    void        ListInit(PacketData *item);
    PacketData *ListCreate();
    PacketData *ListGet();
    void        ListRelease(PacketData *head);
    PacketData **ListGetTail(PacketData **head);

    uint8_t    *ListDataGet(uint32_t *size, const PacketData *item) const;
};