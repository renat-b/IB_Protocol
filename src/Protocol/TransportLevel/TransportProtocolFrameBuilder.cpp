#include "memory.h"
#include "TransportProtocolFrameBuilder.h"
#include "Protocol/CommonLibEmPulse/crc16.h"

TransportProtocolFrameBuilder::TransportProtocolFrameBuilder()
{

}

TransportProtocolFrameBuilder::~TransportProtocolFrameBuilder()
{

}

void TransportProtocolFrameBuilder::SetParams(const ToolAddress *source, const ToolAddress *destination)
{
    m_frame_num  = 0;

    m_address_source      = *source;
    m_address_destination = *destination;
}

void TransportProtocolFrameBuilder::SetNotify(TransportProtocolFrameBuilderNotify notify, const void *param)
{
    m_notify = notify;
    m_notify_param = param;
}

void TransportProtocolFrameBuilder::Initialize()
{
    m_last_error = LAST_ERROR_SUCCESS;
    m_data_offset = 0;
    m_frame_num++;
}

bool TransportProtocolFrameBuilder::Create(const uint8_t *body, uint32_t size, uint32_t flags)
{
    m_last_error = LAST_ERROR_SUCCESS;

    memset(&m_header, 0, sizeof(m_header));
    // инициализируем заголовок пакета
    m_header.version       = TRANSPORT_LEVEL_VERSION;
    m_header.header_length = sizeof(TransportProtocolHeader);

    m_header.address_source      = m_address_source;
    m_header.address_destination = m_address_destination;
    m_header.frame_num   = m_frame_num;

    m_header.flags       = flags;
    m_header.crc8_header = 0;

    m_header.data_length = size;
    m_header.data_offset = m_data_offset;
    m_header.data_crc    = get_crc_16(0, body, size);
    
    m_data_offset += size;

    if (m_notify)
        m_notify(m_notify_param, &m_header, body);
    return true;
}

uint32_t TransportProtocolFrameBuilder::GetLastError() const
{
    return m_last_error;
}
