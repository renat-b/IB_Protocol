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

bool TransportLevelParser::ParseData(const uint8_t *data, uint32_t size)
{
    if (!data || !size)
        return false;



    uint32_t   pos = 0;
    uint32_t   len = 0;
    TransportLevelData *item = ListGet();

    while (size)
    {
        if (!item)
            return false;

        if (m_state == STATE_HEADER)
        {
            len = sizeof(IndigoBaseTransportHeader) - item->data_readed;
            if (len > size)
                len = size;
                
            memcpy(&item->header + item->data_readed, data + pos, len);

            item->data_readed += len; 
            if (item->data_readed == sizeof(IndigoBaseTransportHeader))
                m_state = STATE_BODY;

            // check crc header
        }
        else if (m_state == STATE_BODY)
        {
            if (!item->data)
            {
                item->data = new(std::nothrow) uint8_t[m_data_max_length];
                if (!item->data)
                    return false;
            }
            uint32_t readed = item->data_readed - sizeof(IndigoBaseTransportHeader);
            len = item->header.data_length - readed;
            if (len > size)
                len = size;

            if (!memcpy(item->data + readed, data + pos, len))
                return false;

            item->data_readed += len;
            if (item->data_readed == item->header.data_length)
            {
                uint16_t crc_data_calc = get_crc_16(0, item->data, item->header.data_length);               
                if (crc_data_calc != item->header.data_crc)
                    return false;

                m_state = STATE_HEADER;
            }
        }

        size -= len;
        pos  += len;
    }

    return true;
}

void TransportLevelParser::Clear()
{
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
    
    return true;
}

TransportLevelParser::TransportLevelData *TransportLevelParser::ListGet()
{
    TransportLevelData *item = m_list;

    while (item)
    {
        if (item->data_readed < item->header.data_length)
            return item;

        item = item->next;
    }

    item = ListCreate();
    return item;
}

TransportLevelParser::TransportLevelData *TransportLevelParser::ListCreate()
{
    TransportLevelData **next = &m_list;
    while (*next)
    {
        next = &(*next)->next;
    }
    
    TransportLevelData *item = new(std::nothrow) TransportLevelData;
    if (!item)
        return NULL;

    memset(item, 0, sizeof(TransportLevelData));

    *next = item;
    return item;
}
