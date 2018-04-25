#pragma once

#include "stdint.h"
#include "stdio.h"
#include "Protocol/Common.h"

typedef bool (*TransportProtocolParserNotify)(const void *param, const TransportProtocolHeader *header, const uint8_t *body);

class TransportProtocolBaseParser
{
protected:
    enum constants
    {
        LAST_ERROR_SUCCESS = 0,
        LAST_ERROR_NOT_ENOUGH_MEMORY,
        LAST_ERROR_NOT_ENOUGH_BUFFER,
        LAST_ERROR_CHECK_CRC_BODY,
        LAST_ERROR_CHECK_CRC_HEADER,
        LAST_ERROR_NULL_POINTER,
        LAST_ERROR_NOTIFY,
    };

protected:
    uint32_t  m_last_error = LAST_ERROR_SUCCESS;
    TransportProtocolParserNotify m_notify = NULL;
    const void   *m_notify_param = NULL;


public:
    uint32_t  GetLastError() const;
    void      SetNotify(TransportProtocolParserNotify notify, const void *param);

protected:
    bool     ValidateHeader(const TransportProtocolHeader *header);
    bool     ValidateBody(const TransportProtocolHeader *header, const uint8_t *body);
};