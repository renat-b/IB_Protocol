#include "TransportProtocollParser.h"
#include "Protocol/CommonLibEmPulse/crc16.h"

TransportProtocollParser::TransportProtocollParser()
{
}

TransportProtocollParser::~TransportProtocollParser()
{
    Shutdown();
}

bool TransportProtocollParser::Initialize()
{
    m_last_error = LAST_ERROR_SUCCESS;

    PacketData **tail = ListGetTail(&m_list_free);
    *tail = m_list;

    m_list  = NULL;
    m_pos   = NULL;
    m_state = STATE_HEADER;

    return true;
}

bool TransportProtocollParser::ParseData(const uint8_t *data, uint32_t size)
{
    m_last_error = LAST_ERROR_SUCCESS;

    if (!data || !size)
        return false;


    uint32_t   pos = 0;
    uint32_t   len = 0;
    while (size)
    {
        PacketData *item = ListGet();

        if (!item)
        {
            m_last_error = LAST_ERROR_NULL_POINTER;
            return false;
        }

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
                {
                    m_last_error = LAST_ERROR_CHECK_CRC_HEADER;
                    return false;   
                }

                m_state = STATE_BODY;
            }
            
        }
        else if (m_state == STATE_BODY)
        {
            // выделим память
            if (!item->data)
            {
                item->data = (uint8_t *)MemoryGet(m_data_max_length);
                if (!item->data)
                {
                    m_last_error = LAST_ERROR_NOT_ENOUGH_MEMORY;
                    return false;
                }
            }

            // прочитанные данные тела сообщения
            uint32_t readed = item->data_readed - sizeof(TransportProtocolHeader);
            // рассчитаем полный размер, который нужно дочитать
            len = item->header.data_length - readed;
            // если считать надо больше чем есть в буфере, читаем сколько можем
            if (len > size)
                len = size;
            
            if (!memcpy(item->data + readed, data + pos, len))
            {
                m_last_error = LAST_ERROR_NULL_POINTER;
                return false;
            }

            item->data_readed += len;
            // данные дочитали, проверим корректность
            if ((item->data_readed - sizeof(TransportProtocolHeader)) == item->header.data_length)
            {
                if (!ValidateBody(item))
                {
                    m_last_error = LAST_ERROR_CHECK_CRC_BODY;
                    return false;
                }
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
    m_last_error = LAST_ERROR_SUCCESS;

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
        {
            m_last_error = LAST_ERROR_NULL_POINTER;
            return false;
        }

        size   -= len;
        readed += len;
        m_pos->data_readed += len;

        if (size)
            m_pos = m_pos->next;
    }

    *read_size = readed;         
    if (readed)
        return true;

    return false;

}

uint32_t TransportProtocollParser::GetLastError()
{
    return m_last_error;
}

void TransportProtocollParser::Shutdown()
{
    MemoryRelease();
    m_list = NULL;
    m_list_free = NULL;
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
        if (item->data_readed < (item->header.data_length + sizeof(TransportProtocolHeader)))
            return item;

        item = item->next;
    }

    item = ListCreate();
    return item;
}

TransportProtocollParser::PacketData *TransportProtocollParser::ListCreate()
{
    PacketData **tail = ListGetTail(&m_list);

    bool is_new_item = true;
    PacketData *item = m_list_free;
    if (item)
    {
        is_new_item = false;
        m_list_free  = item->next;
    }
    else
    {
        item = (PacketData *)MemoryGet(sizeof(PacketData));
        if (!item)
        {
            m_last_error = LAST_ERROR_NOT_ENOUGH_MEMORY;
            return NULL;
        }
    }

    memset(&item->header, 0, sizeof(TransportProtocolHeader));
    item->data_readed = 0;
    item->next        = 0;
    if (is_new_item)
        item->data = NULL; 

    *tail = item;
    return item;
}

TransportProtocollParser::PacketData **TransportProtocollParser::ListGetTail(PacketData **head)
{
    PacketData **tail = head;
    while (*tail)
    {
        tail = &(*tail)->next;
    }
    return tail;
}

void *TransportProtocollParser::MemoryGet(uint32_t size)
{
    if ((m_buffer_len + size) > MAX_BUFFER)
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_MEMORY;
        return NULL;
    }

    uint8_t *ptr = m_buffer + m_buffer_len;
    m_buffer_len += size;
    return ptr;
}

void TransportProtocollParser::MemoryRelease()
{

}
