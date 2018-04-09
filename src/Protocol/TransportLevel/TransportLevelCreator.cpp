#include "TransportLevelCreator.h"

TransportLevelCreator::TransportLevelCreator()
{

}

TransportLevelCreator::~TransportLevelCreator()
{

}

bool TransportLevelCreator::AddData(const uint8_t *data, uint32_t size)
{
    if (!data || !size)
        return false;

    if (m_state == STATE_BEGIN)
    {
        if (!Initialize())
            return false;
    }

    if (m_state == STATE_CREATOR)
    {

    }
    return true;
}

bool TransportLevelCreator::Initialize()
{
    Clear();
    
    m_header.version       = TRANSPORT_LEVEL_VERSION;
    m_header.header_length = sizeof(IndigoBaseTransportHeader) - sizeof(m_header.data_offset);
    m_header.data_length   = data_length;

    m_state                = STATE_CREATOR;
    return true;
}

void TransportLevelCreator::Clear()
{
    m_buffer.Clear();
    memset(&m_header, 0, sizeof(m_header));
}