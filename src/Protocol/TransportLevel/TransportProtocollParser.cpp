#include "TransportProtocollParser.h"
#include "Protocol/CommonLibEmPulse/crc16.h"

TransportProtocollParser::TransportProtocollParser()
{
}

TransportProtocollParser::~TransportProtocollParser()
{
    Shutdown();
}

bool TransportProtocollParser::Init()
{
    Shutdown();    

    m_pos   = nullptr;
    m_state = STATE_HEADER;

    return true;
}

bool TransportProtocollParser::ParseData(const uint8_t *data, uint32_t size)
{
    if (!data || !size)
        return false;


    uint32_t   pos = 0;
    uint32_t   len = 0;
    while (size)
    {
        PacketData *item = ListGet();

        if (!item)
            return false;

        if (m_state == STATE_HEADER)
        {
            len = sizeof(TransportProtocolHeader) - item->data_readed;
            if (len > size)
                len = size;
                
            memcpy(&item->header + item->data_readed, data + pos, len);

            item->data_readed += len; 
            if (item->data_readed == sizeof(TransportProtocolHeader))
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
            uint32_t readed = item->data_readed - sizeof(TransportProtocolHeader);
            // рассчитаем полный размер, который нужно дочитать
            len = item->header.data_length - readed;
            // если считать надо больше чем есть в буфере, читаем сколько можем
            if (len > size)
                len = size;
            
            if (!memcpy(item->data + readed, data + pos, len))
                return false;

            item->data_readed += len;
            // данные дочитали, проверим корректность
            if ((item->data_readed - sizeof(TransportProtocolHeader)) == item->header.data_length)
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

bool TransportProtocollParser::GetFirstData(uint8_t *data, uint32_t *read_size)
{
    // сбросим размер прочитанных данных у всех узлов
    m_pos = m_list;
    while (m_pos)
    {
        m_pos->data_readed = 0;
        m_pos = m_pos->next;
    }

    m_pos = m_list;

    bool r = GetNextData(data, read_size);
    return r;
}

bool TransportProtocollParser::GetNextData(uint8_t *data, uint32_t *read_size)
{
    if (!data || !read_size || !*read_size)
        return false;


    uint32_t  readed = 0;
    uint32_t  len = 0;
    uint32_t  size = *read_size;
    // до тех пор пока есть емкость буфера и есть откуда читать
    while (size && m_pos)
    {
        // определим размер, который нужно считать
        len = m_pos->header.data_length - m_pos->data_readed;
        if (len > size)
            len = size;

        if (!memcpy(data + readed, m_pos->data + m_pos->data_readed, len))
            return false;

        size   -= len;
        readed += len;
        m_pos->data_readed += len;

        if (size)
            m_pos = m_pos->next;
    }

    *read_size = readed;         
    return true;

}

void TransportProtocollParser::Shutdown()
{
    PacketData *item = m_list;
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

bool TransportProtocollParser::ValidateHeader(const TransportProtocolHeader *header) const
{
    if (header->version != TRANSPORT_LEVEL_VERSION)
        return false;
    
    if (header->header_length != sizeof(TransportProtocolHeader))
        return false;

    // check crc8
    return true;
}

bool TransportProtocollParser::ValidateBody(const PacketData *item) const
{
    uint16_t crc_data_calc = get_crc_16(0, item->data, item->header.data_length);               
    if (crc_data_calc != item->header.data_crc)
        return false;
    
    return true;
}

TransportProtocollParser::PacketData *TransportProtocollParser::ListGet()
{
    PacketData *item = m_list;

    while (item)
    {
        if (item->data_readed < item->header.data_length)
            return item;

        item = item->next;
    }

    item = ListCreate();
    return item;
}

TransportProtocollParser::PacketData *TransportProtocollParser::ListCreate()
{
    PacketData **next = &m_list;
    while (*next)
    {
        next = &(*next)->next;
    }
    
    PacketData *item = new(std::nothrow) PacketData;
    if (!item)
        return NULL;

    memset(item, 0, sizeof(PacketData));

    *next = item;
    return item;
}
