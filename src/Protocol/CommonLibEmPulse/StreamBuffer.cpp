#include "StreamBuffer.h"

StreamBuffer::StreamBuffer() : m_data(nullptr), m_size(0), m_cur(nullptr)
{
}

StreamBuffer::~StreamBuffer()
{

}

bool StreamBuffer::Initialize(const uint8_t *data, int size)
{
    if (!data)
        return false;
    if (!size)
        return false;
    
    m_data = (uint8_t *)data;
    m_size = size;

    m_cur  = m_data;
    return true;
}

void StreamBuffer::Clear()
{
    m_data = nullptr;
    m_cur  = nullptr;
    m_size = 0;
}

int StreamBuffer::GetPos()
{
    return m_cur - m_data;
}

int StreamBuffer::GetRemaind()
{
    int ret;

    ret = m_size - GetPos();
    return ret;
}

bool StreamBuffer::IsEOF()
{
    return GetRemaind() < 1;
}

bool StreamBuffer::GetInt32(uint32_t *val)
{
    if (GetRemaind() < sizeof(*val))
        return false;

    memcpy(val, m_cur, sizeof(*val));
    m_cur += sizeof(*val);
    return true;
}

bool StreamBuffer::GetInt16(uint16_t *val)
{
    if (GetRemaind() < sizeof(*val))
        return false;

    memcpy(val, m_cur, sizeof(*val));
    m_cur += sizeof(*val);
    return true;
}

bool StreamBuffer::GetRawData(void *val, uint32_t size)
{
    if (GetRemaind() < (int)size)
        return false;

    memcpy(val, m_cur, size);
    m_cur += size;
    return true;
}

uint8_t* StreamBuffer::GetRawDataPtr(uint32_t size)
{
    if (GetRemaind() < (int)size)
        return false;
    
    uint8_t *ptr = m_cur;
    m_cur       += size;
    return ptr;
}

bool StreamBuffer::GetInt8(uint8_t *val)
{
    if (GetRemaind() < sizeof(*val))
        return false;

    memcpy(val, m_cur, sizeof(*val));
    m_cur += sizeof(*val);
    return true;
}



StreamFile::StreamFile()
{
}

StreamFile::~StreamFile()
{
}

bool StreamFile::Initialize(const char *path)
{
    if (!m_buffered_file.FileOpen(path))
        return false;
    return true;
}

void StreamFile::Clear()
{
}

int StreamFile::GetRemaind()
{
    return m_buffered_file.GetRemaind();
}

bool StreamFile::IsEOF()
{
    return m_buffered_file.IsEOF();
}

bool StreamFile::GetRawData(void *val, uint32_t size)
{
    uint8_t *ptr;

    bool r = m_buffered_file.Next((uint8_t**)&ptr, size);
    if (!r)
        return false;

    if (!memcpy(val, ptr, size))
        return false;

    return true;
}

uint8_t *StreamFile::GetRawDataPtr(uint32_t size)
{
    uint8_t *ptr = nullptr;

    bool r = m_buffered_file.Next(&ptr, size);
    if (!r)
        return nullptr;
    return ptr;
}

bool StreamFile::GetInt32(uint32_t *val)
{
    uint32_t *tmp = 0;

    if (!m_buffered_file.Next((uint8_t **)&tmp, sizeof(uint32_t)))
        return false;

    *val = *tmp;
    return true;
}

bool StreamFile::GetInt16(uint16_t *val)
{
    uint16_t *tmp = 0;

    if (!m_buffered_file.Next((uint8_t **)&tmp, sizeof(uint16_t)))
        return false;

    *val = *tmp;
    return true;
}

bool StreamFile::GetInt8(uint8_t *val)
{
    uint8_t *tmp = 0;

    if (!m_buffered_file.Next(&tmp, sizeof(uint8_t)))
        return false;

    *val = *tmp;
    return true;
}
