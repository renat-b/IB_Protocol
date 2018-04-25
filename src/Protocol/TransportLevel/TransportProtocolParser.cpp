#include "TransportProtocolParser.h"
#include "string.h"
#include "Protocol/CommonLibEmPulse/crc16.h"


TransportProtocolParser::TransportProtocolParser()
{
}

TransportProtocolParser::~TransportProtocolParser()
{
}

bool TransportProtocolParser::Initialize()
{
    m_last_error = LAST_ERROR_SUCCESS;
    m_buffer_pos = 0;
    memset(m_buffer, 0, sizeof(m_buffer));
    return true;
}

bool TransportProtocolParser::Parse(const uint8_t *data, uint32_t size)
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

bool TransportProtocolParser::ParseHeader(const uint8_t **data, uint32_t *size)
{
    // ��� ������� ���������
    if (m_buffer_pos >= sizeof(TransportProtocolHeader))
        return true;
    
    // ���������� ������� ����� �������� ������
    uint32_t readed = sizeof(TransportProtocolHeader) - m_buffer_pos;
    if (readed > *size)
        readed = *size;

    // �������� ����� � ������ �������?
    if ((TRANSPORT_MAX_SIZE_FRAME - m_buffer_pos) < readed)
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_BUFFER;
        return false;
    }

    // �������� ������ � �����
    if (!memcpy(m_buffer + m_buffer_pos, *data, readed))
    {
        m_last_error = LAST_ERROR_NULL_POINTER;
        return false;
    }
    
    // ������������ �� ������ ��������� ������
    (*data)      += readed;
    *size        -= readed; 
    m_buffer_pos += readed;
    
    // ��������� �������, �������� ���
    if (m_buffer_pos >= sizeof(TransportProtocolHeader))
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

bool TransportProtocolParser::ParseBody(const uint8_t **data, uint32_t *size)
{
    if (!m_header)
    {
        m_last_error = LAST_ERROR_NULL_POINTER;
        return false;
    }

    uint32_t read = *size;
    // �������� ������� ����� �������� ���� ���������
    uint32_t need_size = m_header->data_length + sizeof(TransportProtocolHeader) - m_buffer_pos;
    // ���� ����� ������ ������ ������ ��� ��������� �������, ������������� 
    if (read > need_size)
        read = need_size;
    
    // �������� ������ ������ �����������?    
    if ((TRANSPORT_MAX_SIZE_FRAME - m_buffer_pos) < read)
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_MEMORY;
        return false;
    }
    
    // �������� ������ 
    if (!memcpy(m_buffer + m_buffer_pos, *data, read))
    {
        m_last_error = LAST_ERROR_NULL_POINTER;
        return false;
    }
    
    // ������������ �� ����������� ������
    *size        -= read;
    (*data)      += read;
    m_buffer_pos += read;
   
    // ���� ��������� ��������� ��������, �������� �������� 
    if ((m_header->data_length + sizeof(TransportProtocolHeader)) == m_buffer_pos)
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
        m_buffer_pos = 0;
        m_header = NULL;
    }
    return true;
}
