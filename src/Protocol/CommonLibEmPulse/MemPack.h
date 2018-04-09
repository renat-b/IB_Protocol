#pragma once

#include "stdint.h"

class MemPack
{
private:
    uint8_t    *m_buffer;
    uint32_t    m_size;
    uint32_t    m_capacity;


public:
    MemPack();
    ~MemPack();
    
    void      Shutdown();
    void      Clear();
    
    bool      Assign(const MemPack &other);
    uint8_t  *GetData() const;
    uint32_t  GetSize() const;
    void      SetSize(uint32_t size);

    bool      AddInt64(uint64_t val);
    bool      AddInt32(uint32_t val);
    bool      AddInt16(uint16_t val);
    bool      AddInt8(uint8_t val);
    bool      Add(const void *val, uint32_t size);
    bool      AddString(const char *val, uint32_t size);
    bool      AddStringFormat(const char *value, ...);

    bool      AddMemPack(MemPack *mem);
    bool      Fill(uint8_t val, uint32_t size);
    bool      Resize(uint32_t add);
};
