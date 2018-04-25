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
    // уже скачали заголовок
    if (m_buffer_pos >= sizeof(TransportProtocolHeader))
        return true;
    
    // рассчитаем сколько нужно докачать данных
    uint32_t readed = sizeof(TransportProtocolHeader) - m_buffer_pos;
    if (readed > *size)
        readed = *size;

    // проверим места в буфере хватает?
    if ((TRANSPORT_MAX_SIZE_FRAME - m_buffer_pos) < readed)
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_BUFFER;
        return false;
    }

    // копируем данные в буфер
    if (!memcpy(m_buffer + m_buffer_pos, *data, readed))
    {
        m_last_error = LAST_ERROR_NULL_POINTER;
        return false;
    }
    
    // корректируем на размер скаченных данных
    (*data)      += readed;
    *size        -= readed; 
    m_buffer_pos += readed;
    
    // заголовок скачали, проверим его
    if (m_buffer_pos >= sizeof(TransportProtocolHeader))
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

bool TransportProtocolParser::ParseBody(const uint8_t **data, uint32_t *size)
{
    if (!m_header)
    {
        m_last_error = LAST_ERROR_NULL_POINTER;
        return false;
    }

    uint32_t read = *size;
    // вычислим сколько нужно докачать тела сообщения
    uint32_t need_size = m_header->data_length + sizeof(TransportProtocolHeader) - m_buffer_pos;
    // если общий размер данных больше чем требуемый остаток, скорректируем 
    if (read > need_size)
        read = need_size;
    
    // проверим размер буфера достаточный?    
    if ((TRANSPORT_MAX_SIZE_FRAME - m_buffer_pos) < read)
    {
        m_last_error = LAST_ERROR_NOT_ENOUGH_MEMORY;
        return false;
    }
    
    // копируем данные 
    if (!memcpy(m_buffer + m_buffer_pos, *data, read))
    {
        m_last_error = LAST_ERROR_NULL_POINTER;
        return false;
    }
    
    // корректируем на прочитанный размер
    *size        -= read;
    (*data)      += read;
    m_buffer_pos += read;
   
    // тело сообщения полностью вычитано, начинаем проверки 
    if ((m_header->data_length + sizeof(TransportProtocolHeader)) == m_buffer_pos)
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
        m_buffer_pos = 0;
        m_header = NULL;
    }
    return true;
}
