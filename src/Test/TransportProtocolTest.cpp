#include "stdio.h"
#include "string.h"
#include "stdarg.h"
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
    /*
    for (body_size = 0; body_size <= 16 * 1024; body_size++)
    {
        PrintLog("body size: %d", body_size);

         if (!CreateBody(body_size))
            return false;

         if (!Run())
            return false;
    }
    */

    for (uint32_t i = 0; i < 10000; i++)
    {
         body_size = (uint32_t)(((double)rand() / RAND_MAX) * ((16 * 1024) - 0)) + 0;       

         if (!CreateBody(body_size))
            return false;
        
         if (!Run())
            return false;

         PrintLog("#%d body size: %d, send frames: %d, windows size counts %d", i, body_size, m_debug_send_frames, m_debug_windows_size_frames);
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

    m_debug_send_frames = 0;
    m_debug_windows_size_frames = 0;

    if (!m_buffer_src.Resize(size))
    {
        PrintLog("failed allocate body, size: %d", size);
        return false;
    }

    for (uint32_t i = 0; i < size; i++)
    {
        uint8_t val = (uint8_t)(((double)rand() / RAND_MAX) * (255 - 0)) + 0;
        if (!m_buffer_src.AddInt8(val))
        {
            PrintLog("failed create body, val: %d", (uint32_t)val);
            return false;
        }
    }
    return true;
}

void TransportProtocolTest::Initalize()
{
    m_builder.SetNotify(&TransportProtocolTest::NotifyBuilder, this);
    m_builder.SetAddress(&m_address, &m_address);

    m_parser.SetNotify(&TransportProtocolTest::NotifyParser, this);

    m_buffer_dst.Resize(17 * 1024);
    m_buffer_src.Resize(17 * 1024);
}

bool TransportProtocolTest::MessagesCreate()
{
    if (!m_builder.Initialize())
    {
        PrintLog("failed initialize codec");
        return false;
    }

    if (!m_parser.Initialize())
    {
        PrintLog("failed initialize decodec");
        return false;
    }


    uint32_t body_size = m_buffer_src.GetSize();
    uint8_t *body = m_buffer_src.GetData();

    uint32_t window_size = 64;
    if (window_size > body_size)
        window_size = body_size;
    
    bool  r;
    uint32_t pos = 0;
    do
    {
        r = m_builder.BodyAdd(body + pos, window_size);
        if (!r)
        {
            PrintLog("failed add body, windows size: %d", window_size);
            break;
        }

        pos += window_size;
        // вычислим следующее рандомное окно - размер
        window_size = (uint32_t)(((double)rand() / RAND_MAX) * (127 - 1)) + 1;
        if ((pos + window_size) > body_size)
            window_size = body_size - pos;

        m_debug_windows_size_frames++;
    }
    while (pos < body_size);

    if (r)
    {
        r = m_builder.BodyEnd();
        if (!r)
            PrintLog("failed create to messages, error: %d", m_builder.GetLastError());
    }

    if (!r)
    {
        if (m_builder.GetLastError())
            PrintLog("failed codec, error: %d", m_builder.GetLastError());
        if (m_parser.GetLastError())
            PrintLog("failed decodec, error: %d", m_parser.GetLastError());
    }

    return r;
}

bool TransportProtocolTest::MessageSend(const uint8_t *data, uint32_t size)
{
    if (!m_parser.Parse(data, size))
    {
        PrintLog("failed add data to decoder, read size: %d, error: %d", size, m_parser.GetLastError());
        return false;
    }
    m_debug_send_frames++;
    return true;
}

bool TransportProtocolTest::MessageRead(const TransportProtocolHeader *header, const uint8_t *body)
{
    if (!m_buffer_dst.Add(body, header->data_length))
    {
        PrintLog("failed add dest buffer, size: %d", header->data_length);
        return false;
    }
    return true;
}

bool TransportProtocolTest::MessagesCheck()
{
    if (m_buffer_dst.GetSize() != m_buffer_src.GetSize())
    {
        PrintLog("incorrect length, size dest: %d, size source: %d", m_buffer_dst.GetSize(), m_buffer_src.GetSize());
        return false;
    }
    for (uint32_t i = 0; i < m_buffer_dst.GetSize(); i++)
    {
        uint8_t dst = m_buffer_dst.GetData()[i];
        uint8_t source = m_buffer_src.GetData()[i];
        if (dst != source)
        {
            PrintLog("incorrect value, dest value: %d, sorce value: %d", (uint32_t)dst, (uint32_t)source);
            return false;
        }
    }
    return true;
}

void TransportProtocolTest::PrintLog(const char *fmt, ...)
{
    if (!fmt)
        return;

    va_list va;
    va_start(va, fmt);

    char buf[128];
    if (vsprintf_s(buf, fmt, va) < 0)
    {
        va_end(va);
        return;
    }
    strcat_s(buf, "\n");
    va_end(va);

    printf(buf);
}

bool TransportProtocolTest::NotifyBuilder(const void *param, const uint8_t *data, uint32_t size)
{
    TransportProtocolTest *self = (TransportProtocolTest *)param;
    if (!self)
        return false;

    if (!self->MessageSend(data, size))
        return false;
    return true;
}

bool TransportProtocolTest::NotifyParser(const void *param, const TransportProtocolHeader *header, const uint8_t *body)
{
    TransportProtocolTest *self = (TransportProtocolTest *)param;
    if (!self)
        return false;

    if (!self->MessageRead(header, body))
        return false;
    return true;
}
