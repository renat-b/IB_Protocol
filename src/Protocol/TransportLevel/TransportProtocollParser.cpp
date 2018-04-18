#include "TransportProtocollParser.h"
#include "Protocol/CommonLibEmPulse/crc16.h"

TransportProtocollParser::TransportProtocollParser()
{
}

TransportProtocollParser::~TransportProtocollParser()
{
}

bool TransportProtocollParser::Initialize()
{
    m_last_error = LAST_ERROR_SUCCESS;
    m_buffer_len = 0;
    m_buffer_pos = 0;
    return true;
}

bool TransportProtocollParser::AddData(const uint8_t *data, uint32_t size)
{
    // ��������, ����� �������� ������, ������� ������� ������?
    if ((m_buffer_len + size) > MAX_BUFFER)
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_MEMORY;
        return false;
    }

    // �������� ������ � �����
    memcpy(m_buffer + m_buffer_len, data, size);
    m_buffer_len += size;
    return true;
}

bool TransportProtocollParser::Parse()
{
    uint32_t pos = 0;
    // �� ��� ��� ���� ���� ������
    while (pos < m_buffer_len)
    {
        // ��������, ��������� ����?
        if ((pos + sizeof(TransportProtocolHeader)) > m_buffer_len)
        {
            m_last_error = LAST_ERROR_NOT_ENOUGH_BUFFER;
            return false;
        }
         
        PacketData *packet = (PacketData *)(m_buffer + pos); 
        // �������� ������������ ���������
        if (!ValidateHeader(&packet->header))
        {
            m_last_error = LAST_ERROR_CHECK_CRC_HEADER;
            return false;
        } 
        
        // �������� ���� ������ ���� ������ ����?
        pos += sizeof(TransportProtocolHeader);
        if ((pos + packet->header.data_length) > m_buffer_len)
        {
            m_last_error = LAST_ERROR_NOT_ENOUGH_BUFFER;
            return false;
        }
        // �������� ������������ ���� ������
        if (!ValidateBody(&packet->header, m_buffer + pos))
        {
            m_last_error = LAST_ERROR_CHECK_CRC_BODY;
            return false;
        }
        pos += packet->header.data_length;
    }
    // ��������� �� ����� ������ ������ ��������� � �������� ������
    if (pos != m_buffer_len)
        return false;
    return true;
}

uint8_t *TransportProtocollParser::GetFirstData(uint32_t *size)
{
    // ������� ��������� �� ������ ������ ���������� �������
    m_last_error = LAST_ERROR_SUCCESS;
    m_buffer_pos = 0;

    uint8_t *data = GetData(size);
    return data;
}

uint8_t *TransportProtocollParser::GetNextData(uint32_t *size)
{
    m_last_error = LAST_ERROR_SUCCESS;

    uint8_t *data = GetData(size);
    return data;
}

uint32_t TransportProtocollParser::GetLastError()
{
    return m_last_error;
}

bool TransportProtocollParser::ValidateHeader(const TransportProtocolHeader *header)
{
    if (header->version != TRANSPORT_LEVEL_VERSION)
        return false;
    
    if (header->header_length != sizeof(TransportProtocolHeader))
        return false;

    // check crc8
    return true;
}

bool TransportProtocollParser::ValidateBody(const TransportProtocolHeader *header, const uint8_t *body)
{
    uint16_t crc_data_calc = get_crc_16(0, body, header->data_length);
    if (crc_data_calc != header->data_crc)
        return false;
    
    return true;
}

uint8_t *TransportProtocollParser::GetData(uint32_t *size)
{
    if (!size)
    {
        m_last_error = LAST_ERROR_NULL_POINTER;
        return NULL;
    }
    // ���� ������ ��������� �� ����� ���������� ������ ����� ������� ������, ������ �� ��������� �������� ������,
    if (m_buffer_pos == m_buffer_len)
        return NULL;

    // �������� ��������� � ������ ����?
    if ((m_buffer_pos + sizeof(TransportProtocolHeader) > m_buffer_len))
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_BUFFER;
        return NULL;
    }


    PacketData *packet = (PacketData *)(m_buffer + m_buffer_pos);
    m_buffer_pos += sizeof(TransportProtocolHeader);
    // �������� ������ ���� ���������
    uint8_t *data = m_buffer + m_buffer_pos;
    // �������� ���� ��������� ����?
    if ((m_buffer_pos + packet->header.data_length) > m_buffer_len)
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_BUFFER;
        return NULL;
    }
    // ������ ������ ���� ������
    *size = packet->header.data_length;
    m_buffer_pos += packet->header.data_length;
    return data;
}
