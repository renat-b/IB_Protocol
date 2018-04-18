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
        r = MessagesSend();
    if (r)
        r = MessagesRead();
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

bool TransportProtocolTest::MessagesCreate()
{
    uint32_t body_size = m_buffer_src.GetSize();
    uint8_t *body = m_buffer_src.GetData();

    m_creator.SetAddress(&m_address, &m_address);
    bool r = m_creator.Initialize();
    if (r)
    {

        uint32_t window_size = 64;
        if (window_size > body_size)
            window_size = body_size;

        uint32_t pos = 0;
        do
        {
            r = m_creator.AddBody(body + pos, window_size);
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
    }
    if (r)
    {
        r = m_creator.Build();
        if (!r)
            printf("failed create to messages, error: %d\n", m_creator.GetLastError());
    }
    return r;
}

bool TransportProtocolTest::MessagesSend()
{
    uint32_t size = 0;
    uint8_t *data = nullptr;

    if (!m_parser.Initialize())
    {
        printf("failed initialize parser\n");
        return false;
    }

    uint32_t readed = 0;
    if ((data = m_creator.GetPacketFirst(&size)))
    {
        do 
        {
            if (!m_parser.AddData(data, size))
            {
                printf("failed add data to parser, readed size: %d, read size: %d, error: %d\n", readed, size, m_parser.GetLastError());
                return false;
            }
            readed += size;
        } 
        while ((data = m_creator.GetPacketNext(&size)));
    }
    return true;
}

bool TransportProtocolTest::MessagesRead()
{
    uint8_t *buf;
    uint32_t readed = sizeof(buf);

    if (!m_parser.Parse())
    {
        printf("failed parse packet, error: %d\n", m_parser.GetLastError());
        return false;
    }

    if ((buf = m_parser.GetFirstData(&readed)))
    {
        do 
        {
            if (!m_buffer_dst.Add(buf, readed))
            {
                printf("failed add buffer destination, size: %d\n", readed);
                return false;
            }
        } 
        while ((buf = m_parser.GetNextData(&readed)));
    }

    if (m_parser.GetLastError())
    {
        printf("failed get data, error: %d\n", m_parser.GetLastError());
        return false;
    }
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
