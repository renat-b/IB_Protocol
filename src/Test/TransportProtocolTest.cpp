#include "stdio.h"
#include "TransportProtocolTest.h"

TransportProtocolTest::TransportProtocolTest()
{
    srand(_time32(NULL));

    m_address.tool_id = 0;
    m_address.serial_num = 0;
}

TransportProtocolTest::~TransportProtocolTest()
{
}

bool TransportProtocolTest::Test()
{
    Initalize();

    uint32_t body_size;
    for (body_size = 0; body_size <= 16 * 1024; body_size++)
    {
         printf("body size: %d\n", body_size);

         if (!CreateBody(body_size))
            return false;

         if (!Run())
            return false;
    }

    for (uint32_t i = 0; i < 10000; i++)
    {
         body_size = (uint32_t)(((double)rand() / RAND_MAX) * ((8 * 1024) - 0)) + 0;       
         printf("#%d body size: %d\n", i, body_size);

         if (!CreateBody(body_size))
            return false;
        
         if (!Run())
            return false;
    }
    return true;
}

bool TransportProtocolTest::Run()
{
    bool r = MessagesCreate(); 
    if (r)
        r = MessagesCheck();
    return r;
}

bool TransportProtocolTest::CreateBody(uint32_t size)
{
    m_buffer_src.Clear();
    m_buffer_dst.Clear();

    if (!m_buffer_src.Resize(size))
    {
        printf("failed allocate body, size: %d\n", size);
        return false;
    }

    for (uint32_t i = 0; i < size; i++)
    {
        uint8_t val = (uint8_t)(((double)rand() / RAND_MAX) * (255 - 0)) + 0;
        if (!m_buffer_src.AddInt8(val))
        {
            printf("failed create body, val: %d", (uint32_t)val);
            return false;
        }
    }
    return true;
}

void TransportProtocolTest::Initalize()
{
    m_codec.SetNotify(this, &TransportProtocolTest::NotifyCodec);
    m_codec.SetAddress(&m_address, &m_address);

    m_decodec.SetNotify(this, &TransportProtocolTest::NotifyDecodec);
}

bool TransportProtocolTest::MessagesCreate()
{
    uint32_t body_size = m_buffer_src.GetSize();
    uint8_t *body = m_buffer_src.GetData();

    if (!m_codec.Initialize())
    {
        printf("failed initialize coder\n");
        return false;
    }

    if (!m_decodec.Initialize())
    {
        printf("failed initialize decoder\n");
        return false;
    }

    uint32_t window_size = 64;
    if (window_size > body_size)
        window_size = body_size;
    
    bool  r;
    uint32_t pos = 0;
    do
    {
        r = m_codec.Create(body + pos, window_size);
        if (!r)
        {
            printf("failed add body, windows size: %d\n", window_size);
            break;
        }

        pos += window_size;
        // вычислим следующее рандомное окно - размер
        window_size = (uint32_t)(((double)rand() / RAND_MAX) * (127 - 1)) + 1;
        if ((pos + window_size) > body_size)
            window_size = body_size - pos;
    }
    while (pos < body_size);

    if (r)
    {
        r = m_codec.CreateEnd();
        if (!r)
            printf("failed create to messages, error: %d\n", m_codec.GetLastError());
    }

    return r;
}

bool TransportProtocolTest::MessageSend(const uint8_t *data, uint32_t size)
{
    if (!m_decodec.Parse(data, size))
    {
        printf("failed add data to decoder, read size: %d, error: %d\n", size, m_decodec.GetLastError());
        return false;
    }
    return true;
}

bool TransportProtocolTest::MessageRead(const TransportProtocolHeader *header, const uint8_t *body)
{
    if (!m_buffer_dst.Add(body, header->data_length))
        return false;

    return true;
}

bool TransportProtocolTest::MessagesCheck()
{
    if (m_buffer_dst.GetSize() != m_buffer_src.GetSize())
    {
        printf("incorrect length, size dest: %d, size source: %d\n", m_buffer_dst.GetSize(), m_buffer_src.GetSize());
        return false;
    }
    for (uint32_t i = 0; i < m_buffer_dst.GetSize(); i++)
    {
        uint8_t dst = m_buffer_dst.GetData()[i];
        uint8_t source = m_buffer_src.GetData()[i];
        if (dst != source)
        {
            printf("incorrect value, dest value: %d, sorce value: %d\n", (uint32_t)dst, (uint32_t)source);
            return false;
        }
    }
    return true;
}

bool TransportProtocolTest::NotifyCodec(void *param, const uint8_t *data, uint32_t size)
{
    TransportProtocolTest *self = (TransportProtocolTest *)param;
    if (!self)
        return false;

    if (!self->MessageSend(data, size))
        return false;
    return true;
}

bool TransportProtocolTest::NotifyDecodec(void *param, const TransportProtocolHeader *header, const uint8_t *body)
{
    TransportProtocolTest *self = (TransportProtocolTest *)param;
    if (!self)
        return false;

    if (!self->MessageRead(header, body))
        return false;
    return true;
}
