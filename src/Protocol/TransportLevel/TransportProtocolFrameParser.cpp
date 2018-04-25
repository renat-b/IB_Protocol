#include "TransportProtocolFrameParser.h"
#include "Protocol/CommonLibEmPulse/crc16.h"

TransportProtocolFrameParser::TransportProtocolFrameParser()
{
}

TransportProtocolFrameParser::~TransportProtocolFrameParser()
{
}

void TransportProtocolFrameParser::Initalize()
{
    m_notify = NULL;
    m_notify_param = NULL;
    m_last_error = LAST_ERROR_SUCCESS;
}

bool TransportProtocolFrameParser::Parse(const uint8_t *data, uint32_t size)
{
    TransportProtocolHeader *header = NULL;

    bool r = ParseHeader(&header, &data, &size);
    if (r)
        r = ParseBody(header, &data, &size);

    return r;
}

bool TransportProtocolFrameParser::ParseHeader(TransportProtocolHeader **header, const uint8_t **data, uint32_t *size)
{
    // рассчитаем сколько нужно считать 
    uint32_t read = sizeof(TransportProtocolHeader);

    // проверим места в буфере хватает?
    if (*size < read)
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_BUFFER;
        return false;
    }

    *header = (TransportProtocolHeader *)(*data);
    if (!ValidateHeader(*header))
    {
        m_last_error = LAST_ERROR_CHECK_CRC_HEADER;
        return false;
    }

    // корректируем на размер считанных данных
    (*data)      += read;
    *size        -= read; 
    return true; 
}

bool TransportProtocolFrameParser::ParseBody(const TransportProtocolHeader *header, const uint8_t **data, uint32_t *size)
{
    uint32_t read = header->data_length;
    // проверим размер буфера достаточный?    
    if (read > *size)
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_MEMORY;
        return false;
    }

    // проверим корректность тела сообщения
    if (!ValidateBody(header, *data))
    {
        m_last_error = LAST_ERROR_CHECK_CRC_BODY;
        return false;
    }

    // уведомим 
    if (m_notify)
    {
        if (!m_notify(m_notify_param, header, *data))
        {
            m_last_error = LAST_ERROR_NOTIFY;
            return false;
        }
    }
   
    // корректируем на прочитанный размер
    *size        -= read;
    (*data)      += read;
    return true;
}