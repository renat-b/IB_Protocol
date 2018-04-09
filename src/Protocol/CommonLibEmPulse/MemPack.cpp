#include "MemPack.h"
#include "new.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdarg.h"

MemPack::MemPack() : m_buffer(nullptr), m_size(0), m_capacity(0)
{

}

MemPack::~MemPack()
{
    Shutdown();
}

void MemPack::Shutdown()
{
    if (m_buffer)
    {
        delete[] m_buffer;
    }
    m_buffer   = nullptr;
    m_size     = 0;
    m_capacity = 0;
}

void MemPack::Clear()
{
    m_size = 0;
}

bool MemPack::Assign(const MemPack &other)
{
    Clear();

    bool r = Add(other.m_buffer, other.m_size);
    return r;
}

uint8_t *MemPack::GetData() const
{
    return m_buffer;
}

uint32_t MemPack::GetSize() const
{
    return m_size;
}

void MemPack::SetSize(uint32_t size)
{
    if (m_capacity < size)
        return;

    m_size = size;
}

bool MemPack::AddInt64(uint64_t val)
{
    bool r = Add(&val, sizeof(val));
    return r;
}

bool MemPack::AddInt32(uint32_t val)
{
    bool r = Add(&val, sizeof(val));
    return r;
}

bool MemPack::AddInt16(uint16_t val)
{
    bool r = Add(&val, sizeof(val));
    return r;
}


bool MemPack::AddInt8(uint8_t val)
{
    bool r = Add(&val, sizeof(val));
    return r;
}

bool MemPack::Add(const void *val, uint32_t size)
{
    if (val == nullptr)
        return false;

    if (!Resize(size))
        return false; 

    memcpy(m_buffer + m_size, val, size);
    m_size += size;
    return true;
}

bool MemPack::AddString(const char *val, uint32_t size)
{
    bool r; 
    r = Add(val, size);
    return r;
}

bool MemPack::AddStringFormat(const char *value, ...)
{
    va_list arg_ptr;
    char    str[1024] = { 0 };
    int     len;

    va_start(arg_ptr, value);
    if ((len = _vsnprintf_s(str, _countof(str) - 1, _TRUNCATE, value, arg_ptr)) < 0)
    {
        va_end(arg_ptr);
        return(false);
    }
    va_end(arg_ptr);

    return(AddString(str, (uint32_t)len));
}

bool MemPack::AddMemPack(MemPack *mempack)
{
    uint8_t *data = mempack->GetData();
    uint32_t size = mempack->GetSize();

    bool r = Add(data, size);
    return r;

}

bool MemPack::Fill(uint8_t val, uint32_t size)
{
    if (!Resize(size))
        return false;

    memset(m_buffer + m_size, val, size);
    m_size += size;
    return true;
}

bool MemPack::Resize(uint32_t size)
{
    uint32_t new_size; 
    new_size = size + m_size;
    if (new_size < m_capacity)
        return true;

    if (new_size < 16)
        new_size += 16;
    else
        new_size = (uint32_t)(new_size * 1.3); 


    uint8_t *buf;
    buf = new(std::nothrow) uint8_t[new_size];
    if (!buf)
        return false;
    
    if (m_buffer)
    {
        memcpy(buf, m_buffer, m_size);
        delete [] m_buffer;
        m_buffer = nullptr;
    } 
    m_buffer   = buf;
    m_capacity = new_size;
    return true;
}
