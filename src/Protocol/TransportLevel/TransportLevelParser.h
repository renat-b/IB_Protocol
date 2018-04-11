#pragma once

#include "stdint.h"
#include "Protocol/Common.h"
#include "Protocol/CommonLibEmPulse/StreamBuffer.h"

class TransportLevelParser
{
private:
    enum constants
    {
        STATE_HEADER = 1,
        STATE_BODY   = 2,
        MAX_SIZE     = 260,
    };

    struct TransportLevelData
    {
        TransportLevelData *next;
        uint32_t            data_readed;
        uint8_t            *data;
        IndigoBaseTransportHeader header;
    };


private:
    uint32_t     m_state     = STATE_HEADER;

    uint32_t     m_data_max_length = MAX_SIZE;
    uint32_t     m_data_offset = 0;

    TransportLevelData *m_list = nullptr;
    TransportLevelData *m_list_free = nullptr;


private:
    StreamBuffer       m_buffer;
    IndigoBaseTransportHeader m_header;

public:
    TransportLevelParser();
    ~TransportLevelParser();

    bool  Parse(const uint8_t *data, uint32_t size);
    bool  ParseData(const uint8_t *data, uint32_t size);

private:
    void  Clear();
    bool  ParseVersion();
    bool  ParseLength();
    bool  ParseAdditionalFields();
    bool  CheckHeader();
    bool  CheckData();

    TransportLevelData   *ListGet();
    TransportLevelData   *ListCreate();
};