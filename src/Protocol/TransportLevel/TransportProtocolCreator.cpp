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
}

void TransportProtocolCreator::SetAddress(const ToolAddress *address_source, const ToolAddress *address_destination)
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

uint32_t TransportProtocolCreator::GetLastError() const
{
    return m_last_error;
}

bool TransportProtocolCreator::Initialize()
{
    m_last_error = LAST_ERROR_SUCCESS;

    m_packet_num = 0; 
    m_buffer_len = 0;
    m_packet_pos = 0;

    m_frame_num++;
    m_data_offset = 0;
    return true;
}

bool TransportProtocolCreator::AddBody(const uint8_t *data, uint32_t size)
{
    m_last_error = LAST_ERROR_SUCCESS;    

    // до тех пор пока есть данные 
    uint32_t pos = 0;
    do 
    {
        PacketData *packet = PacketGet();
        if (!packet)
            return false;

        uint32_t len = size;
        uint32_t free_size = TRANSPORT_MAX_SIZE_PACKET - sizeof(TransportProtocolHeader) - packet->header.data_length;
        // смотрим, данные целиком войдут в узел, или придется дробить данные
        if (size > free_size)
            len = free_size;
        
        // копируем данные в буфер узла 
        uint8_t *dst = (uint8_t *)packet + sizeof(TransportProtocolHeader);
        if (!memcpy(dst + packet->header.data_length, data + pos, len))
        {
            m_last_error = LAST_ERROR_NULL_POINTER;
            return false;
        } 

        // сместим все данные на величину len
        packet->header.data_length += len;
        pos           += len;
        size          -= len;
        m_data_offset += len;
    } while (size);
    return true;

}

bool TransportProtocolCreator::Build()
{
    uint32_t pos = 0;
    while (pos < m_packet_num)
    {
        PacketData *packet = (PacketData *)(m_buffer + TRANSPORT_MAX_SIZE_PACKET * pos);
        uint8_t *data = (uint8_t *)packet + sizeof(TransportProtocolHeader);       

        // рассчитаем crc для данных
        packet->header.data_crc = get_crc_16(0, data, packet->header.data_length);
        // рассчитаем crc для заголовка
        packet->header.crc8_header = 0;
        // для мульти узлов
        if (m_packet_num > 1)
        {
            if (pos < (m_packet_num - 1))
                packet->header.flags = TRANSPORT_FLAG_FRAGMENTATION;
            else // последний узел?
                packet->header.flags = TRANSPORT_FLAG_FRAGMENTATION_LAST;
        }
        pos++;
    }
    return true;
}

uint8_t *TransportProtocolCreator::GetPacketFirst(uint32_t *size)
{
    if (!size)
        return NULL;

    *size        = 0;
    m_packet_pos = 0;

    if (m_packet_pos >= m_packet_num)
        return NULL;
    
    uint8_t    *data   = m_buffer + m_packet_pos * TRANSPORT_MAX_SIZE_PACKET;
    PacketData *packet = (PacketData *)data;

    *size = sizeof(TransportProtocolHeader) + packet->header.data_length;
    m_packet_pos++;
    return data;
}

uint8_t *TransportProtocolCreator::GetPacketNext(uint32_t *size)
{
    if (!size)
        return NULL;

    *size = 0;
    if (m_packet_pos >= m_packet_num)
        return NULL;
    
    uint8_t    *data   = m_buffer + m_packet_pos * TRANSPORT_MAX_SIZE_PACKET;
    PacketData *packet = (PacketData *)data;

    *size = sizeof(TransportProtocolHeader) + packet->header.data_length;
    m_packet_pos++;
    return data;
}

TransportProtocolCreator::PacketData *TransportProtocolCreator::PacketGet()
{
    uint32_t max_packets = MAX_BUFFER / TRANSPORT_MAX_SIZE_PACKET;
    if (m_packet_num > 0)
    {
        if (m_packet_num > max_packets)
            return NULL;

        PacketData *packet = (PacketData *)(m_buffer + (m_packet_num - 1) * TRANSPORT_MAX_SIZE_PACKET);
        if ((packet->header.data_length + sizeof(TransportProtocolHeader)) < TRANSPORT_MAX_SIZE_PACKET)
            return packet;
    }

    m_packet_num++;
    if (m_packet_num > max_packets)
        return NULL;

    PacketData *packet = (PacketData *)(m_buffer + (m_packet_num - 1) * TRANSPORT_MAX_SIZE_PACKET);
    memset(packet, 0, TRANSPORT_MAX_SIZE_PACKET);

    packet->header.version       = TRANSPORT_LEVEL_VERSION;
    packet->header.header_length = sizeof(TransportProtocolHeader);

    packet->header.address_source      = m_address_source;
    packet->header.address_destination = m_address_destination;
    packet->header.frame_num   = m_frame_num;

    packet->header.flags       = 0;
    packet->header.crc8_header = 0;

    packet->header.data_length = 0;
    packet->header.data_offset = m_data_offset;
    packet->header.data_crc    = 0;
    
    return packet;    
}