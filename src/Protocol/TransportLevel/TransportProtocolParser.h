#pragma once

#include "TransportProtocolBaseParser.h"

class TransportProtocolParser : public TransportProtocolBaseParser
{
private:
    enum constants
    {
        STATE_HEADER = 1,
        STATE_BODY   = 2,
    };

private:
    uint8_t      m_buffer[TRANSPORT_MAX_SIZE_FRAME];
    uint32_t     m_buffer_pos = 0;
    
    uint32_t     m_state = STATE_HEADER;
    TransportProtocolHeader *m_header;


public:
    TransportProtocolParser();
    ~TransportProtocolParser();

    bool     Initialize();
    bool     Parse(const uint8_t *data, uint32_t size);


private:
    bool     ParseHeader(const uint8_t **data, uint32_t *size);
    bool     ParseBody(const uint8_t **data, uint32_t *size);
};