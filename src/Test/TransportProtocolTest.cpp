#include "TransportProtocolTest.h"

TransportProtocolTest::TransportProtocolTest()
{
    m_address.tool_id = 0;
    m_address.serial_num = 0;
}

TransportProtocolTest::~TransportProtocolTest()
{
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
    if (!m_buffer_src.Resize(size))
        return false;

    for (uint32_t i = 0; i < size; i++)
        m_buffer_src.AddInt8(i);

    return true;
}

bool TransportProtocolTest::MessagesCreate()
{
    uint32_t body_size = m_buffer_src.GetSize();
    uint8_t *body = m_buffer_src.GetData();

    m_creator.SetAddress(&m_address, &m_address);
    bool r = m_creator.Init();
    if (r)
    {
        srand(_time32(NULL));
        uint32_t window_size = 64;
        for (uint32_t pos = 0; pos < body_size;)
        {
            r = m_creator.AddBody(body + pos, window_size);
            if (!r)
                break;

            pos += window_size;
            // вычислим следующее рандомное окно - размер
            window_size = (uint32_t)(((double)rand() / RAND_MAX) * (128 - 64)) + 64;
            if ((window_size + pos) > body_size)
                window_size = body_size - pos;
        }
    }
    if (r)
        r = m_creator.Stop();

    return r;
}

bool TransportProtocolTest::MessagesSend()
{
    uint32_t size = 0;
    uint8_t *data = nullptr;

    if (!m_parser.Init())
        return false;

    if ((data = m_creator.PacketFirst(&size)))
    {
        do 
        {
            if (!MessageSend(data, size))
                return false;
        } 
        while ((data = m_creator.PacketNext(&size)));
    }
    return true;
}

bool TransportProtocolTest::MessageSend(const uint8_t *data, uint32_t size)
{
    if (!m_parser.ParseData(data, size))
        return false;

    return true;
}

bool TransportProtocolTest::MessagesRead()
{
    uint8_t buf[64];
    uint32_t readed = sizeof(buf);

    if (m_parser.GetFirstData(buf, &readed) && readed)
    {
        do 
        {
            if (!m_buffer_dst.Add(buf, readed))
                return false;

            readed = sizeof(buf);
        } 
        while (m_parser.GetNextData(buf, &readed) && readed != 0);
    }
    return true;
}

bool TransportProtocolTest::MessagesCheck()
{
    if (m_buffer_dst.GetSize() != m_buffer_src.GetSize())
        return false;

    for (uint32_t i = 0; i < m_buffer_dst.GetSize(); i++)
    {
        if (m_buffer_src.GetData()[i] != m_buffer_dst.GetData()[i])
            return false;
    }
    return true;
}
