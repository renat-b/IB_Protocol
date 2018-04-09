#include "TransportLevelCreator.h"
#include "string.h"

TransportLevelCreator::TransportLevelCreator()
{

}

TransportLevelCreator::~TransportLevelCreator()
{

}

void TransportLevelCreator::SetParams(uint32_t max_chunk_size)
{
    m_max_size = max_chunk_size;
}

bool TransportLevelCreator::DataAdd(const uint8_t *data, uint32_t size)
{
    if (!data || !size)
        return false;

    if (m_state == STATE_BEGIN)
    {
    }

    if (m_state == STATE_CREATOR)
    {

    }
    return true;
}

void TransportLevelCreator::Clear()
{
    m_buffer.Clear();
    memset(&m_header, 0, sizeof(m_header));
}