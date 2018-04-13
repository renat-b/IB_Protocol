#include "string.h"
#include "new.h"
#include "stddef.h"
#include "TransportProtocolCreator.h"
#include "Protocol/CommonLibEmPulse/crc16.h"

TransportProtocolCreator::TransportProtocolCreator()
{
    m_address_source.tool_id    = 0;
    m_address_source.serial_num = 0;

    m_address_destination.tool_id    = 0;
    m_address_destination.serial_num = 0;
}

TransportProtocolCreator::~TransportProtocolCreator()
{
    m_data_max_length = 0;
    m_data_offset     = 0;

    ListRelease(m_list);
    m_list = NULL;

    ListRelease(m_list_free);
    m_list_free = NULL;
}

void TransportProtocolCreator::SetAddress(const ToolAddress *address_source, const ToolAddress *address_destination)
{
    if (!address_source || !address_destination)
        return;

    m_address_source      = *address_source;
    m_address_destination = *address_destination;
}

bool TransportProtocolCreator::Init()
{
    m_frame_num++;
    m_data_offset = 0;
   
    
    PacketData **list = &m_list_free;
    // ищем хвост в списке свободных чанков
    while (*list)
    {
        list = &(*list)->next;
    }
    // освободим список занятых чанков
    *list  = m_list;
    m_list = NULL;
    return true;
}

bool TransportProtocolCreator::AddBody(const uint8_t *data, uint32_t size)
{
    uint32_t pos = 0;

    if (!data || !size)
        return false;

    // до тех пор пока есть данные 
    while (size)
    {
        PacketData *item = ListGet();
        if (!item)
            return false;
 
        uint32_t len = size;
        // смотрим, данные целиком войдут в узел, или придется дробить данные
        if (size > (m_data_max_length - item->header.data_length))
            len = m_data_max_length - item->header.data_length;
        
        // копируем данные в буфер узла 
        if (!memcpy(item->data + item->header.data_length, data + pos, len))
            return false;
        
        // если новый узел (оффсет не выставлен) выставим правильное смещение 
        if ( !(m_flags & FLAGS_START_OFFSET))
        {
            item->header.data_offset = m_data_offset;
            m_flags |= FLAGS_START_OFFSET;
        } 
        // сместим все данные на величину len
        item->header.data_length += len;
        pos           += len;
        size          -= len;
        m_data_offset += len;
    }
    return true;
}

uint8_t *TransportProtocolCreator::PacketFirst(uint32_t *size)
{
    if (!size)
        return NULL;

    *size = 0;
    m_pos = m_list;
    if (!m_pos)
        return NULL;

    uint8_t *ptr = ListDataGet(size, m_pos); 
    return ptr;
}

uint8_t *TransportProtocolCreator::PacketNext(uint32_t *size)
{
    if (!size)
        return NULL;
    if (!m_pos)
        return NULL;

    *size = 0;
    m_pos = m_pos->next;
    if (!m_pos)
        return NULL;

    uint8_t *ptr = ListDataGet(size, m_pos); 
    return ptr;
}

bool TransportProtocolCreator::Stop()
{
    if (!m_list)
        return false;


    bool is_multi = m_list->next ? true : false;
    PacketData *item = m_list;

    while (item)
    {
        // рассчитаем crc для данных
        item->header.data_crc = get_crc_16(0, item->data, item->header.data_length);
        // рассчитаем crc для заголовка
        item->header.crc8_header = 0;
        // для мульти узлов
        if (is_multi)
        {
            if (item->next)
                item->header.flags = TRANSPORT_LEVEL_FLAG_FRAGMENTATION;
            else // последний узел?
                item->header.flags = TRANSPORT_LEVEL_FLAG_FRAGMENTATION_LAST;
        }

        item = item->next;
    }
    return true;
}

bool TransportProtocolCreator::ListInit(PacketData *item)
{
    if (!item)
        return false;

    item->header.version       = TRANSPORT_LEVEL_VERSION;
    item->header.header_length = sizeof(TransportProtocolHeader);

    item->header.address_source      = m_address_source;
    item->header.address_destination = m_address_destination;
    item->header.frame_num   = m_frame_num;

    item->header.flags       = 0;
    item->header.crc8_header = 0;

    item->header.data_length = 0;
    item->header.data_offset = 0;
    item->header.data_crc    = 0;

    item->next = NULL;
    m_flags &= FLAGS_START_OFFSET;
    return true;
}

TransportProtocolCreator::PacketData *TransportProtocolCreator::ListCreate()
{
    // пытаемся выделить чанк из списка свободных чанков
    PacketData **next = &m_list;
    // ищем последний пустой элемент
    while (*next)
    {
        next = &((*next)->next);
    }

    PacketData *list = m_list_free;
    if (list)
    {
        m_list_free = list->next;
        *next = list;
        return list;
    }
    
    // аллоцируем новый элемент
    // рассчитаем общий размер данных
    uint32_t len = sizeof(PacketData) + m_data_max_length;
    // выделим память под данные
    uint8_t *ptr = new(std::nothrow) uint8_t[len];
    if (!ptr)
        return NULL;

    list = (PacketData *)ptr;
    list->data = ptr + sizeof(PacketData);

    *next = list;
    return list;
}

TransportProtocolCreator::PacketData *TransportProtocolCreator::ListGet()
{
    PacketData *item = m_list;
    while (item)
    {
        if (item->header.data_length < m_data_max_length)
            return item;
        item = item->next;
    }

    item = ListCreate();
    if (item)
        ListInit(item);
    return item;
}

void TransportProtocolCreator::ListRelease(PacketData *head)
{
    uint8_t *ptr;
    PacketData *next;

    while (head)
    {
        next = head->next;

        ptr = (uint8_t *)head;
        delete[] ptr;
        head = next;
    }
}

uint8_t *TransportProtocolCreator::ListDataGet(uint32_t *size, const PacketData *item) const
{
    uint8_t *ptr = (uint8_t *)item;
    ptr  += offsetof(PacketData, header);

    *size = sizeof(TransportProtocolHeader) + item->header.data_length;
    return ptr;
}
