#include "TransportProtocolBaseParser.h"
#include "Protocol/CommonLibEmPulse/crc16.h"

uint32_t TransportProtocolBaseParser::GetLastError() const
{
    return m_last_error;
}

void TransportProtocolBaseParser::SetNotify(TransportProtocolParserNotify notify, const void *param)
{
    m_notify = notify;
    m_notify_param = param;
}

bool TransportProtocolBaseParser::ValidateHeader(const TransportProtocolHeader *header)
{
    if (header->version != TRANSPORT_LEVEL_VERSION)
        return false;
    
    if (header->header_length != sizeof(TransportProtocolHeader))
        return false;

    // проверим, размер буфера меньше тела сообщения, должен быть не больше размера буфера
    if (header->data_length > TRANSPORT_MAX_SIZE_BODY)
        return false;

    // check crc8
    return true;
}

bool TransportProtocolBaseParser::ValidateBody(const TransportProtocolHeader *header, const uint8_t *body)
{
    uint16_t crc_data_calc = get_crc_16(0, body, header->data_length);
    if (crc_data_calc != header->data_crc)
        return false;
    
    return true;
}