#include "TransportProtocolDecodec.h"
#include "Protocol/CommonLibEmPulse/crc16.h"

TransportProtocolDecodec::TransportProtocolDecodec()
{
}

TransportProtocolDecodec::~TransportProtocolDecodec()
{
}

bool TransportProtocolDecodec::Initialize()
{
    m_last_error = LAST_ERROR_SUCCESS;
    m_buffer_len = 0;
    memset(m_buffer, 0, sizeof(m_buffer));
    return true;
}

void TransportProtocolDecodec::SetNotify(void *param, TransportProtocolDecodecNotify notify)
{
    m_notify_param = param;
    m_notify = notify;
}

bool TransportProtocolDecodec::Parse(const uint8_t *data, uint32_t size)
{
    m_last_error = LAST_ERROR_SUCCESS;

    do 
    {
        if (m_state == STATE_HEADER)
        {
            if (!ParseHeader(&data, &size))
                return false;
        }

        if (m_state == STATE_BODY)
        {
            if (!ParseBody(&data, &size))
                return false;
        }

    } while (size);

    return true;
}

uint32_t TransportProtocolDecodec::GetLastError()
{
    return m_last_error;
}

bool TransportProtocolDecodec::ValidateHeader(const TransportProtocolHeader *header)
{
    if (header->version != TRANSPORT_LEVEL_VERSION)
        return false;
    
    if (header->header_length != sizeof(TransportProtocolHeader))
        return false;

    // ��������, ������ ������ ������ ���� ���������, ������ ���� �� ������ ������� ������
    if (header->data_length > (m_buffer_max - sizeof(TransportProtocolHeader)))
        return false;

    // check crc8
    return true;
}

bool TransportProtocolDecodec::ValidateBody(const TransportProtocolHeader *header, const uint8_t *body)
{
    uint16_t crc_data_calc = get_crc_16(0, body, header->data_length);
    if (crc_data_calc != header->data_crc)
        return false;
    
    return true;
}

bool TransportProtocolDecodec::ParseHeader(const uint8_t **data, uint32_t *size)
{
    // ��� ������� ���������
    if (m_buffer_len >= sizeof(TransportProtocolHeader))
        return true;
    
    // ���������� ������� ����� �������� ������
    uint32_t read = sizeof(TransportProtocolHeader) - m_buffer_len;
    if (read > *size)
        read = *size;

    // �������� ����� � ������ �������?
    if ((m_buffer_max - m_buffer_len) < read)
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_BUFFER;
        return false;
    }

    // �������� ������ � �����
    if (!memcpy(m_buffer + m_buffer_len, *data, read))
    {
        m_last_error = LAST_ERROR_NULL_POINTER;
        return false;
    }
    
    // ������������ �� ������ ��������� ������
    (*data)      += read;
    *size        -= read; 
    m_buffer_len += read;
    
    // ��������� �������, �������� ���
    if (m_buffer_len >= sizeof(TransportProtocolHeader))
    {
        m_header = (TransportProtocolHeader *)m_buffer;
        if (!ValidateHeader(m_header))
        {
            m_last_error = LAST_ERROR_CHECK_CRC_HEADER;
            return false;
        }
        // ����� ���������� ���� ���������
        m_state = STATE_BODY;
    } 

    return true; 
}

bool TransportProtocolDecodec::ParseBody(const uint8_t **data, uint32_t *size)
{
    if (!m_header)
    {
        m_last_error = LAST_ERROR_NULL_POINTER;
        return false;
    }

    uint32_t read = *size;
    // �������� ������� ����� �������� ���� ���������
    uint32_t need_size = m_header->data_length + sizeof(TransportProtocolHeader) - m_buffer_len;
    // ���� ����� ������ ������ ������ ��� ��������� �������, ������������� 
    if (read > need_size)
        read = need_size;
    
    // �������� ������ ������ �����������?    
    if ((m_buffer_max - m_buffer_len) < read)
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_MEMORY;
        return false;
    }
    
    // �������� ������ 
    if (!memcpy(m_buffer + m_buffer_len, *data, read))
    {
        m_last_error = LAST_ERROR_NULL_POINTER;
        return false;
    }
    
    // ������������ �� ����������� ������
    *size        -= read;
    (*data)      += read;
    m_buffer_len += read;
   
    // ���� ��������� ��������� ��������, �������� �������� 
    if ((m_header->data_length + sizeof(TransportProtocolHeader)) == m_buffer_len)
    {
        uint8_t *body = m_buffer + sizeof(TransportProtocolHeader);
        // �������� ������������ ���� ���������
        if (!ValidateBody(m_header, body))
        {
            m_last_error = LAST_ERROR_CHECK_CRC_BODY;
            return false;
        }

        // �������� 
        if (m_notify)
        {
            if (!m_notify(m_notify_param, m_header, body))
            {
                m_last_error = LAST_ERROR_NOTIFY;
                return false;
            }
        }

        m_state = STATE_HEADER;
        m_buffer_len = 0;
        m_header = NULL;
    }
    return true;
}
