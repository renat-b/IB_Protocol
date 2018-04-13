#include "TransportLevelParser.h"
#include "Protocol/CommonLibEmPulse/crc16.h"

TransportLevelParser::TransportLevelParser()
{
}

TransportLevelParser::~TransportLevelParser()
{
    Shutdown();
}

bool TransportLevelParser::ParseData(const uint8_t *data, uint32_t size)
{
    if (!data || !size)
        return false;



    uint32_t   pos = 0;
    uint32_t   len = 0;
    while (size)
    {
        TransportLevelData *item = ListGet();

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
            {
                if (!ValidateHeader(&item->header))
                    return false;   

                m_state = STATE_BODY;
            }
            
        }
        else if (m_state == STATE_BODY)
        {
            // выделим память
            if (!item->data)
            {
                item->data = new(std::nothrow) uint8_t[m_data_max_length];
                if (!item->data)
                    return false;
            }

            // прочитанные данные тела сообщения
            uint32_t readed = item->data_readed - sizeof(IndigoBaseTransportHeader);
            // рассчитаем полный размер, который нужно дочитать
            len = item->header.data_length - readed;
            // если считать надо больше чем есть в буфере, читаем сколько можем
            if (len > size)
                len = size;
            
            if (!memcpy(item->data + readed, data + pos, len))
                return false;

            item->data_readed += len;
            // данные дочитали, проверим корректность
            if ((item->data_readed - sizeof(IndigoBaseTransportHeader)) == item->header.data_length)
            {
                if (!ValidateBody(item))
                    return false;

                m_state = STATE_HEADER;
            }
        }

        size -= len;
        pos  += len;
    }

    return true;
}

void TransportLevelParser::Shutdown()
{
    TransportLevelData *item = m_list;
    while (item)
    {
        m_list = item->next;
        if (item->data)
            delete[] item->data;
        delete item;
        item = m_list;
    }
    m_list = NULL;
}

bool TransportLevelParser::ValidateHeader(const IndigoBaseTransportHeader *header) const
{
    if (header->version != TRANSPORT_LEVEL_VERSION)
        return false;
    
    if (header->header_length != sizeof(IndigoBaseTransportHeader))
        return false;

    // check crc8
    return true;
}

bool TransportLevelParser::ValidateBody(const TransportLevelData *item) const
{
    uint16_t crc_data_calc = get_crc_16(0, item->data, item->header.data_length);               
    if (crc_data_calc != item->header.data_crc)
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
