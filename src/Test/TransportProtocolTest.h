#pragma once

#include "new.h"
#include "stdint.h"
#include "stdlib.h"
#include "time.h"
#include "Protocol/Common.h"
#include "Protocol/TransportLevel/TransportProtocolCreator.h"
#include "Protocol/TransportLevel/TransportProtocollParser.h"
#include "Protocol/CommonLibEmPulse/MemPack.h"


class TransportProtocolTest
{
private:
    MemPack  m_buffer_src;
    MemPack  m_buffer_dst;
    ToolAddress m_address;
    TransportProtocolCreator m_creator;
    TransportProtocollParser m_parser;

public:
    TransportProtocolTest();
    ~TransportProtocolTest();

    bool  Test();
    bool  Run();
    bool  CreateBody(uint32_t size);

private:
    bool  MessagesCreate();
    bool  MessagesSend();
    bool  MessagesRead();
    bool  MessagesCheck();
};