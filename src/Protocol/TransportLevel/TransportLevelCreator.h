#pragma once

#include "stdint.h"
#include "Protocol/Common.h"
#include "Protocol/CommonLibEmPulse/MemPack.h"

class TransportLevelCreator
{
private:
    enum constants
    {
        STATE_BEGIN   = 1,
        STATE_CREATOR = 2,

        MAX_SIZE  = 260,
    };

    struct TransportLevelData
    {
        TransportLevelData *next;
        uint8_t            *data;
        IndigoBaseTransportHeader header;
    };


private:
    uint16_t     m_frame_num = 0;

    uint32_t     m_data_max_length = 0;
    uint32_t     m_data_length   = 0;
    uint32_t     m_data_offset   = 0;

    uint8_t     *m_buffer  = nullptr;
    TransportLevelData *m_list      = nullptr;
    TransportLevelData *m_list_free = nullptr;
    
    ToolAddress m_address_source;
    ToolAddress m_address_destination;


public:
    TransportLevelCreator();
    ~TransportLevelCreator();

    bool    BufferCreate(uint32_t chunk_size);
    void    AddressSet(const ToolAddress *address_source, const ToolAddress *address_destination);

    bool    DataStart();
    bool    DataAdd(const uint8_t *data, uint32_t size);
    bool    DataEnd();

private:
    bool    ListAssign();
    TransportLevelData *ListGet();
    void    ListRelease(TransportLevelData *head);
};