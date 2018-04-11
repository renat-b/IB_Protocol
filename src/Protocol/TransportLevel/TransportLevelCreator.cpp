#include "string.h"
#include "new.h"
#include "TransportLevelCreator.h"
#include "Protocol/CommonLibEmPulse/crc16.h"

TransportLevelCreator::TransportLevelCreator()
{
    m_address_source.tool_id    = 0;
    m_address_source.serial_num = 0;

    m_address_destination.tool_id    = 0;
    m_address_destination.serial_num = 0;
}

TransportLevelCreator::~TransportLevelCreator()
{
    m_data_max_length = 0;
    m_data_offset     = 0;

    ListRelease(m_list);
    m_list = NULL;

    ListRelease(m_list_free);
    m_list_free = NULL;
}

void TransportLevelCreator::SetAddress(const ToolAddress *address_source, const ToolAddress *address_destination)
{
    if (!address_source || !address_destination)
        return;

    m_address_source      = *address_source;
    m_address_destination = *address_destination;
}

bool TransportLevelCreator::Start()
{
    m_frame_num++;
    m_data_offset = 0;
   
    
    TransportLevelData **list = &m_list_free;
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

bool TransportLevelCreator::AddData(const uint8_t *data, uint32_t size)
{
    uint32_t pos = 0;

    if (!data || !size)
        return false;

    // до тех пор пока есть данные 
    while (size)
    {
        TransportLevelData *item = ListGet();
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
        if (!item->header.data_offset)
            item->header.data_offset = m_data_offset;
        
        // сместим все данные на величину len
        item->header.data_length += len;
        pos           += len;
        size          -= len;
        m_data_offset += len;
    }
    return true;
}

bool TransportLevelCreator::End()
{
    uint8_t flags = 0;
    TransportLevelData *item = m_list;
    
    if (item->next)
        flags = TRANSPORT_LEVEL_FLAG_FRAGMENTATION;

    while (item)
    {
        // рассчитаем crc для данных
        item->header.data_crc = get_crc_16(0, item->data, item->header.data_length);
        // рассчитаем crc для заголовка
        item->header.header_length = 0;
        // для мульти узлов
        if (flags)
        {
            item->header.flags = flags;
            // последний узел?
            if (!item->next)
                item->header.flags |= TRANSPORT_LEVEL_FLAG_FRAGMENTATION_LAST;
        }

        item = item->next;
    }
    return true;
}

bool TransportLevelCreator::ListInit(TransportLevelData *item)
{
    if (!item)
        return false;

    item->header.version       = TRANSPORT_LEVEL_VERSION;
    item->header.header_length = sizeof(IndigoBaseTransportHeader);

    item->header.address_source      = m_address_source;
    item->header.address_destination = m_address_destination;
    item->header.frame_num   = m_frame_num;

    item->header.flags       = 0;
    item->header.crc8_header = 0;

    item->header.data_length = 0;
    item->header.data_offset = 0;
    item->header.data_crc    = 0;

    return true;
}

TransportLevelCreator::TransportLevelData *TransportLevelCreator::ListCreate()
{
    // пытаемся выделить чанк из списка свободных чанков
    TransportLevelData **next = &m_list;
    // ищем последний пустой элемент
    while (!*next)
    {
        next = &((*next)->next);
    }

    TransportLevelData *list = m_list_free;
    if (list)
    {
        m_list_free = list->next;
        *next = list;
        return list;
    }
    
    // аллоцируем новый элемент
    // рассчитаем общий размер данных
    uint32_t len = sizeof(TransportLevelData) + m_data_max_length;
    // выделим память под данные
    uint8_t *ptr = new(std::nothrow) uint8_t[len];
    if (!ptr)
        return NULL;

    list = (TransportLevelData *)ptr;
    list->data = ptr + sizeof(TransportLevelData);

    *next = list;
    return list;
}

TransportLevelCreator::TransportLevelData *TransportLevelCreator::ListGet()
{
    TransportLevelData *item = m_list;
    while (item)
    {
        if (item->header.data_length < m_data_max_length)
            return item;
    }

    item = ListCreate();
    if (item)
        ListInit(item);
    return item;
}

void TransportLevelCreator::ListRelease(TransportLevelData *head)
{
    uint8_t *ptr;
    TransportLevelData *next;

    while (head)
    {
        next = head->next;

        ptr = (uint8_t *)head;
        delete[] ptr;
        head = next;
    }
}