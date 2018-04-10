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
    if (m_buffer)
    {
        delete[] m_buffer;
        m_buffer = NULL;
    }

    m_data_max_length = 0;
    m_data_length     = 0;
    m_data_offset     = 0;

    ListRelease(m_list);
    m_list = NULL;

    ListRelease(m_list_free);
    m_list_free = NULL;
}

bool TransportLevelCreator::BufferCreate(uint32_t chunk_size)
{
    if (chunk_size <= sizeof(IndigoBaseTransportHeader))
        return false;
    
    if (m_buffer)
    {
        delete[] m_buffer;
        m_buffer = nullptr;
    }

    m_data_max_length = chunk_size - sizeof(IndigoBaseTransportHeader);
    m_buffer = new(std::nothrow) uint8_t[m_data_max_length];
    if (!m_buffer)
        return false;
     
    m_data_length = 0;    
    return true;
}

void TransportLevelCreator::AddressSet(const ToolAddress *address_source, const ToolAddress *address_destination)
{
    if (!address_source || !address_destination)
        return;

    m_address_source = *address_source;
    m_address_destination = *address_destination;
}

bool TransportLevelCreator::DataStart()
{
    m_frame_num++;
    m_data_length = 0;
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

bool TransportLevelCreator::DataAdd(const uint8_t *data, uint32_t size)
{
    if (!data || !size)
        return false;
       
    uint32_t len = size;
    if (size > (m_data_max_length - m_data_length))
        len = m_data_max_length - m_data_length;

    if (len)
    {
        if (!memcpy(m_buffer + m_data_length, data, len))
            return false;
        m_data_length += len;
    }
    if (!(size - len))
        return true;

    ListAssign(); 
    return true; 
}

bool TransportLevelCreator::DataEnd()
{
    return true;
}

bool TransportLevelCreator::ListAssign()
{
    TransportLevelData *list = ListGet();
    if (!list)
        return false;

    list->header.version = TRANSPORT_LEVEL_VERSION;
    list->header.header_length = sizeof(IndigoBaseTransportHeader);
    list->header.address_source      = m_address_source;
    list->header.address_destination = m_address_destination;
    list->header.frame_num = m_frame_num;
    list->header.flags = 0;
    list->header.crc8_header = 0;

    list->header.data_length = m_data_length;
    list->header.data_offset = m_data_offset;
    list->header.data_crc = 0;

    memcpy(list->data, m_buffer, m_data_length);

    //
    m_data_offset += m_data_length;
    m_data_length  = 0;
    memset(m_buffer, 0, m_data_max_length);
    return true;
}

TransportLevelCreator::TransportLevelData *TransportLevelCreator::ListGet()
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