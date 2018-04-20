#pragma once

#include "new.h"
#include "stdint.h"
#include "stdlib.h"
#include "time.h"
#include "Protocol/Common.h"
#include "Protocol/TransportLevel/TransportProtocolCodec.h"
#include "Protocol/TransportLevel/TransportProtocolDecodec.h"
#include "Protocol/CommonLibEmPulse/MemPack.h"


class TransportProtocolTest
{
private:
    MemPack  m_buffer_src;
    MemPack  m_buffer_dst;
    ToolAddress  m_address;
    TransportProtocolCodec   m_codec;
    TransportProtocolDecodec m_decodec;


public:
    TransportProtocolTest();
    ~TransportProtocolTest();

    bool  Test();
    bool  Run();
    bool  CreateBody(uint32_t size);

private:
    void  Initalize();
    bool  MessagesCreate();
    bool  MessageSend(const uint8_t *data, uint32_t size);
    bool  MessageRead(const TransportProtocolHeader *header, const uint8_t *body);
    bool  MessagesCheck();
    
private:
    static bool  NotifyCodec(void *param, const uint8_t *data, uint32_t size);
    static bool  NotifyDecodec(void *param, const TransportProtocolHeader *header, const uint8_t *body);
};