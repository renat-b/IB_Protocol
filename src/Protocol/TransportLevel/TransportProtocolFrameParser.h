#pragma once

#include "stdint.h"
#include "stdio.h"
#include "Protocol/Common.h"
#include "TransportProtocolBaseParser.h"

class TransportProtocolFrameParser : public TransportProtocolBaseParser
{
public:
    TransportProtocolFrameParser();
    ~TransportProtocolFrameParser();

    void     Initalize();
    bool     Parse(const uint8_t *data, uint32_t size);


private:
    bool     ParseHeader(TransportProtocolHeader **header, const uint8_t **data, uint32_t *size);
    bool     ParseBody(const TransportProtocolHeader *header, const uint8_t **data, uint32_t *size);
};