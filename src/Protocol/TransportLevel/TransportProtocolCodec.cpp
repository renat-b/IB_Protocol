#include "string.h"
#include "new.h"
#include "stddef.h"
#include "TransportProtocolCodec.h"
#include "Protocol/CommonLibEmPulse/crc16.h"

TransportProtocolCodec::TransportProtocolCodec()
{
    m_address_source.tool_id    = 0;
    m_address_source.serial_num = 0;

    m_address_destination.tool_id    = 0;
    m_address_destination.serial_num = 0;
}

TransportProtocolCodec::~TransportProtocolCodec()
{
}

void TransportProtocolCodec::SetAddress(const ToolAddress *address_source, const ToolAddress *address_destination)
{
    m_last_error = LAST_ERROR_SUCCESS; 
    if (!address_source || !address_destination)
    {
        m_last_error = LAST_ERROR_NULL_POINTER;
        return;
    }

    m_address_source      = *address_source;
    m_address_destination = *address_destination;
}

void TransportProtocolCodec::SetNotify(void *param, TransportProtocolCoderNotify notify)
{
    m_notify_param = param;
    m_notify = notify;
}

uint32_t TransportProtocolCodec::GetLastError() const
{
    return m_last_error;
}

bool TransportProtocolCodec::Initialize()
{
    m_last_error = LAST_ERROR_SUCCESS;

    // выставим новый номер пакета
    m_frame_num++;
    m_data_offset = 0;

    memset(m_buffer, 0, m_buffer_max);

    PacketData *packet = PacketGet();
    // инициализируем заголовок пакета
    packet->header.version       = TRANSPORT_LEVEL_VERSION;
    packet->header.header_length = sizeof(TransportProtocolHeader);

    packet->header.address_source      = m_address_source;
    packet->header.address_destination = m_address_destination;
    packet->header.frame_num   = m_frame_num;

    packet->header.flags       = 0;
    packet->header.crc8_header = 0;

    packet->header.data_length = 0;
    packet->header.data_offset = 0;
    packet->header.data_crc    = 0;
    
    return true;
}

bool TransportProtocolCodec::Create(const uint8_t *data, uint32_t size)
{
    //
    m_last_error = LAST_ERROR_SUCCESS;    

    // до тех пор пока есть данные 
    uint32_t pos = 0;
    do 
    {
        PacketData *packet = PacketGet();
        if (!packet)
        {
            m_last_error = LAST_ERROR_NULL_POINTER;
            return false;
        } 

        uint32_t write_size = size;
        // получим размер свободного места в пакете
        uint32_t free_size = PacketSizeFree(packet);
        // пакет заполнен
        if (free_size == 0)
        {
            if (!PacketNotify(packet, TRANSPORT_FLAG_FRAGMENTATION))
            {
                m_last_error = LAST_ERROR_NOTIFY;
                return false;
            }

            // пересчитаем свободное место под пакет
            free_size = PacketSizeMax();
            // выставим новое смещение для следущего пакета
            packet->header.data_offset = m_data_offset;
            packet->header.data_length = 0;
        }

        // смотрим, данные целиком войдут в пакет, или придется дробить данные
        if (write_size > free_size)
            write_size = free_size;
        
        // копируем данные в буфер
        uint8_t *dst = PacketGetData(packet);
        if (!memcpy(dst + packet->header.data_length, data + pos, write_size))
        {
            m_last_error = LAST_ERROR_NULL_POINTER;
            return false;
        } 

        // сместим все данные на величину записанных данных
        packet->header.data_length += write_size;
        pos           += write_size;
        size          -= write_size;
        m_data_offset += write_size;
    } while (size);

    return true;
}

bool TransportProtocolCodec::CreateEnd()
{
    PacketData *packet = PacketGet();
    if (!packet)
        return false;
    
    // определимся, сообщения состоит из одного фрагмента или дробленное?
    uint32_t flags = packet->header.data_offset == 0 ? 0 : TRANSPORT_FLAG_FRAGMENTATION_LAST;   
    if (!PacketNotify(packet, flags))
    {
        m_last_error = LAST_ERROR_NOTIFY;
        return false;
    }
    return true;
}

TransportProtocolCodec::PacketData *TransportProtocolCodec::PacketGet()
{
    PacketData *packet = (PacketData *)m_buffer;
    return packet;    
}

bool TransportProtocolCodec::PacketNotify(PacketData *packet, uint32_t flags)
{
    const uint8_t *data = PacketGetData(packet);
    // рассчитаем crc для данных
    packet->header.data_crc = get_crc_16(0, data, packet->header.data_length);
    // рассчитаем crc для заголовка
    packet->header.crc8_header = 0;
    packet->header.flags = flags;

    // уведомим, что пакет готов
    if (m_notify)
    {
        uint32_t length = sizeof(TransportProtocolHeader) + packet->header.data_length;
        if (!m_notify(m_notify_param, m_buffer, length))
            return false;
    }
    return true;
}

/*
возвращаем указатель на начало тела сообщения
*/
uint8_t *TransportProtocolCodec::PacketGetData(const PacketData *packet)
{
    uint8_t *dst = (uint8_t *)packet + sizeof(TransportProtocolHeader);
    return dst;
}

/*
максимальный размер тела сообщения, который может быть
*/
uint32_t TransportProtocolCodec::PacketSizeMax()
{
    uint32_t max_body_size = m_buffer_max - sizeof(TransportProtocolHeader);
    return max_body_size;
}

/*
свободный размер тела сообщения в который можно записать/ дозаписать данные
*/
uint32_t TransportProtocolCodec::PacketSizeFree(const PacketData *packet)
{
    uint32_t free_size = m_buffer_max - sizeof(TransportProtocolHeader) - packet->header.data_length;
    return free_size;
}