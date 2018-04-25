#include "string.h"
#include "new.h"
#include "stddef.h"
#include "TransportProtocolBuilder.h"
#include "Protocol/CommonLibEmPulse/crc16.h"

TransportProtocolBuilder::TransportProtocolBuilder()
{
    m_builder.SetNotify(&TransportProtocolCodeNotifyFunc, this);
}

TransportProtocolBuilder::~TransportProtocolBuilder()
{
}

void TransportProtocolBuilder::SetAddress(const ToolAddress *address_source, const ToolAddress *address_destination)
{
    if (!address_source || !address_destination)
        return;
    m_builder.SetParams(address_source, address_destination);
}

void TransportProtocolBuilder::SetNotify(TransportProtocolBuilderNotify notify, const void *param)
{
    m_notify_param = param;
    m_notify = notify;
}

uint32_t TransportProtocolBuilder::GetLastError() const
{
    return m_last_error;
}

bool TransportProtocolBuilder::Initialize()
{
    m_builder.Initialize();

    m_buffer_in_pos = 0;
    m_first_frame   = true;
    return true;
}

bool TransportProtocolBuilder::BodyAdd(const uint8_t *body, uint32_t size)
{
    if (!body)
        return false;

    uint32_t write_bytes = 0;
    do 
    {
        write_bytes = size;
        if (write_bytes > (TRANSPORT_MAX_SIZE_BODY - m_buffer_in_pos))
            write_bytes = TRANSPORT_MAX_SIZE_BODY - m_buffer_in_pos;

        if (!memcpy(m_buffer_in + m_buffer_in_pos, body, write_bytes))
        {
            m_last_error = LAST_ERROR_NULL_POINTER;
            return false;
        }

        m_buffer_in_pos += write_bytes;
        body += write_bytes;
        size -= write_bytes;
        
        // отправляем пакет, только в том случае, когда буфер заполнен и остались еще данные
        // в противном случае мы не знаем, это последний пакет или нет
        if ((m_buffer_in_pos >= TRANSPORT_MAX_SIZE_BODY) && size)
        {
            if (!m_builder.Create(m_buffer_in, TRANSPORT_MAX_SIZE_BODY, TRANSPORT_FLAG_FRAGMENTATION))
            {
                m_last_error = m_builder.GetLastError();
                return false;
            }
            m_first_frame = false;
            m_buffer_in_pos = 0;
        }

    } while (size);
    return true;
}

bool TransportProtocolBuilder::BodyEnd()
{
    uint32_t flags = m_first_frame ? 0 : TRANSPORT_FLAG_FRAGMENTATION_LAST; 
    if (!m_builder.Create(m_buffer_in, m_buffer_in_pos, flags))
    {
        m_last_error = m_builder.GetLastError();
        return false;
    }

    m_buffer_in_pos = 0;
    return true;
}

bool TransportProtocolBuilder::BuildFrame(const TransportProtocolHeader *header, const uint8_t *body)
{
    uint32_t writed = header->data_length + header->header_length;
    if (writed > TRANSPORT_MAX_SIZE_FRAME)
        return false;
    
    if (!memcpy(m_buffer_out, header, header->header_length))
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_MEMORY;
        return false;
    }
    if (!memcpy(m_buffer_out + header->header_length, body, header->data_length))
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_MEMORY;
        return false;
    }
    if (m_notify)
    {
        m_notify(m_notify_param, m_buffer_out, writed);
    }
    return true;
}

bool TransportProtocolBuilder::TransportProtocolCodeNotifyFunc(const void *param, const TransportProtocolHeader *header, const uint8_t *body)
{
    if (!param)
        return false;

    TransportProtocolBuilder *self = (TransportProtocolBuilder *)param;
    bool r = self->BuildFrame(header, body);
    return r;
}
