#pragma once

#include "stdint.h"
#include "windows.h"
#include "BufferedFile.h"


class IStreamBuffer
{
public:
    virtual int    GetRemaind() = 0;
    virtual bool   IsEOF() = 0;

    virtual bool   GetRawData(void *val, uint32_t size) = 0;
    virtual uint8_t* GetRawDataPtr(uint32_t size) = 0;

    // конвертация данных
    virtual bool   GetInt32(uint32_t *val) = 0;
    virtual bool   GetInt16(uint16_t *val) = 0;
    virtual bool   GetInt8(uint8_t *val) = 0;
};


class StreamBuffer : public IStreamBuffer
{
private:
    uint8_t *m_data;
    uint8_t *m_cur;
    int      m_size;

public:
    StreamBuffer();
    ~StreamBuffer();

    bool   Initialize(const uint8_t *data, int size);
    void   Clear();

    virtual int    GetRemaind() override;
    virtual bool   IsEOF() override;

    virtual bool   GetRawData(void *val, uint32_t size) override;
    virtual uint8_t* GetRawDataPtr(uint32_t size) override;

    // конвертация данных
    virtual bool   GetInt32(uint32_t *val) override;
    virtual bool   GetInt16(uint16_t *val) override;
    virtual bool   GetInt8(uint8_t *val) override;

private:
    int    GetPos();
};


class StreamFile : public IStreamBuffer
{
private:
    BufferedFile   m_buffered_file;

public:
    StreamFile();
    ~StreamFile();

    bool   Initialize(const char *path);
    void   Clear();

    virtual int    GetRemaind() override;
    virtual bool   IsEOF() override;

    virtual bool   GetRawData(void *val, uint32_t size) override;
    virtual uint8_t *GetRawDataPtr(uint32_t size) override;

    // конвертация данных
    virtual bool   GetInt32(uint32_t *val) override;
    virtual bool   GetInt16(uint16_t *val) override;
    virtual bool   GetInt8(uint8_t *val) override;
};
