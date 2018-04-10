#pragma once

#include "stdint.h"
#include "Protocol/Common.h"
#include "Protocol/CommonLibEmPulse/StreamBuffer.h"

class TransportLevelParser
{
private:
    struct DataTransmission
    {
        uint8_t   *data;
        uint32_t   length;
    };

private:
    StreamBuffer       m_buffer;
    DataTransmission   m_data;
    IndigoBaseTransportHeader m_header;

public:
    TransportLevelParser();
    ~TransportLevelParser();

    bool  Parse(const uint8_t *data, uint32_t size);

private:
    void  Clear();
    bool  ParseVersion();
    bool  ParseLength();
    bool  ParseAdditionalFields();
    bool  CheckHeader();
    bool  CheckData();
};