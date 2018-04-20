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

    // проверим, размер буфера меньше тела сообщения, должен быть не больше размера буфера
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
    // уже скачали заголовок
    if (m_buffer_len >= sizeof(TransportProtocolHeader))
        return true;
    
    // рассчитаем сколько нужно докачать данных
    uint32_t read = sizeof(TransportProtocolHeader) - m_buffer_len;
    if (read > *size)
        read = *size;

    // проверим места в буфере хватает?
    if ((m_buffer_max - m_buffer_len) < read)
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_BUFFER;
        return false;
    }

    // копируем данные в буфер
    if (!memcpy(m_buffer + m_buffer_len, *data, read))
    {
        m_last_error = LAST_ERROR_NULL_POINTER;
        return false;
    }
    
    // корректируем на размер скаченных данных
    (*data)      += read;
    *size        -= read; 
    m_buffer_len += read;
    
    // заголовок скачали, проверим его
    if (m_buffer_len >= sizeof(TransportProtocolHeader))
    {
        m_header = (TransportProtocolHeader *)m_buffer;
        if (!ValidateHeader(m_header))
        {
            m_last_error = LAST_ERROR_CHECK_CRC_HEADER;
            return false;
        }
        // далее обработаем тело сообщения
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
    // вычислим сколько нужно докачать тела сообщения
    uint32_t need_size = m_header->data_length + sizeof(TransportProtocolHeader) - m_buffer_len;
    // если общий размер данных больше чем требуемый остаток, скорректируем 
    if (read > need_size)
        read = need_size;
    
    // проверим размер буфера достаточный?    
    if ((m_buffer_max - m_buffer_len) < read)
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_MEMORY;
        return false;
    }
    
    // копируем данные 
    if (!memcpy(m_buffer + m_buffer_len, *data, read))
    {
        m_last_error = LAST_ERROR_NULL_POINTER;
        return false;
    }
    
    // корректируем на прочитанный размер
    *size        -= read;
    (*data)      += read;
    m_buffer_len += read;
   
    // тело сообщения полностью вычитано, начинаем проверки 
    if ((m_header->data_length + sizeof(TransportProtocolHeader)) == m_buffer_len)
    {
        uint8_t *body = m_buffer + sizeof(TransportProtocolHeader);
        // проверим корректность тела сообщения
        if (!ValidateBody(m_header, body))
        {
            m_last_error = LAST_ERROR_CHECK_CRC_BODY;
            return false;
        }

        // уведомим 
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
