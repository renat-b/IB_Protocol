#pragma once

#include "stdlib.h"
#include "Protocol/Common.h"


typedef bool(*TransportProtocolFrameBuilderNotify) (const void *param, const TransportProtocolHeader *header, const uint8_t *body);

class TransportProtocolFrameBuilder
{
private:
    enum constants
    {
        MAX_BUFFER = TRANSPORT_MAX_SIZE_FRAME,

        LAST_ERROR_SUCCESS = 0,
        LAST_ERROR_NOT_ENOUGH_MEMORY,
        LAST_ERROR_NULL_POINTER,
        LAST_ERROR_NOTIFY,
    };

private:
    uint16_t    m_frame_num    = 0;
    uint32_t    m_last_error   = LAST_ERROR_SUCCESS;
    uint32_t    m_data_offset = 0;
 
    ToolAddress m_address_source;
    ToolAddress m_address_destination;
   
   
    TransportProtocolHeader m_header;
    TransportProtocolFrameBuilderNotify m_notify = NULL;
    const void  *m_notify_param = NULL;


public:
    TransportProtocolFrameBuilder();
    ~TransportProtocolFrameBuilder();

    void   SetParams(const ToolAddress *source, const ToolAddress *destination);
    void   SetNotify(TransportProtocolFrameBuilderNotify notify, const void *param);

    void   Initialize();
    bool   Create(const uint8_t *body, uint32_t size, uint32_t flags);

    uint32_t  GetLastError() const;
};