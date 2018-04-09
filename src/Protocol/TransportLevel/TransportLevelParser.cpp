#include "TransportLevelParser.h"
#include "Protocol/CommonLibEmPulse/crc16.h"

TransportLevelParser::TransportLevelParser()
{
    Clear();
}

TransportLevelParser::~TransportLevelParser()
{
    Clear();
}

bool TransportLevelParser::Parse(const uint8_t *data, uint32_t size)
{
    Clear();
    if (!m_buffer.Initialize(data, size))
        return false;
    
    if (!ParseVersion())
        return false;

    if (!ParseLength())
        return false;

    if (!ParseAdditionalFields())
        return false; 

    if (!CheckHeader())
        return false;

    if (!CheckData())
        return false;

    return true;
}

void TransportLevelParser::Clear()
{
    m_data.data   = NULL;
    m_data.length = 0;
    m_buffer.Clear();
    memset(&m_header, 0, sizeof(m_header));
}

bool TransportLevelParser::ParseVersion()
{
    uint8_t version = 0; 

    if (!m_buffer.GetInt8(&version))
        return false;

    if (version != TRANSPORT_LEVEL_VERSION)
        return false;
    
    m_header.version = version;
    return true;
}

bool TransportLevelParser::ParseLength()
{
    uint8_t length = 0;
    if (!m_buffer.GetInt8(&length))
        return false;

    if (length >= m_buffer.GetRemaind())
        return false;
    
    m_header.header_length = length;
    return true;
}

bool TransportLevelParser::ParseAdditionalFields()
{
    uint32_t length = m_header.header_length;
    uint32_t required_max_length = sizeof(IndigoBaseTransportHeader);
    uint32_t required_min_length = required_max_length - sizeof(m_header.data_offset);

    if (length < required_min_length || length > required_max_length)
        return false;

    length -= sizeof(m_header.version) + sizeof(m_header.header_length);
    if (!m_buffer.GetRawData(&m_header.address_source, length))
        return false;

    return true;
}

bool TransportLevelParser::CheckHeader()
{
    return true;
}

bool TransportLevelParser::CheckData()
{
    uint16_t crc_data;
    if (!m_buffer.GetInt16(&crc_data))
        return false;

    if (m_header.data_length != m_buffer.GetRemaind())
        return false;   


    uint32_t data_length = m_buffer.GetRemaind(); 
    uint8_t *data = m_buffer.GetRawDataPtr(data_length);
    if (!data)
        return false;

    uint16_t crc_calc_data = get_crc_16(0, data, data_length);
    if (crc_data != crc_calc_data)
        return false;
    
    m_data.data   = data;
    m_data.length = data_length;
    return true;
}
