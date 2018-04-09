#include "BufferedFile.h"

bool BufferedFile::FileChunk::Resize(uint32_t new_max_size)
{

    // если размера буфера хватает, выходим - выделять ничего не надо
    if (new_max_size < max_size)
        return true;

    uint8_t *buf = NULL;
    uint32_t capacity = new_max_size / 4;

    // ограничим размер нового буфера по минимальному и максимальному значению
    if (capacity < 16)
        capacity = 16;

    if (capacity > 1024 * 1024)
        capacity = 32 * 1024;

    // новая емкость буфера
    capacity += new_max_size;

    // выделим память под буфер
    buf = new(std::nothrow) uint8_t[capacity];
    if (!buf)
        return false;

    // копируем старые данные
    memcpy(buf, data, size);
    max_size = capacity;

    // освободим старую память и запомним новый буфер
    delete data;
    data = buf;

    return true;
}

void BufferedFile::FileChunk::Free()
{
    if (data)
        delete data;

    data = nullptr;
    size = 0;
    max_size = 0;
}


BufferedFile::BufferedFile()
{

}

BufferedFile::~BufferedFile()
{
    Shutdown();
}

bool BufferedFile::FileOpen(const char *file_name)
{
    if (!file_name)
        return false;
    
    Shutdown();

    m_file = CreateFile(file_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_file == INVALID_HANDLE_VALUE)
        return false;

    m_file_chunk.file_remaind = FileSize();
    return true;
}

/*
*/
bool BufferedFile::Next(uint8_t **data, uint32_t len)
{
    // данные есть? - возвращаем указатель на данные
    if (len <= m_file_chunk.remaind)
    {
        *data = m_file_chunk.data + m_file_chunk.pos;

        m_file_chunk.pos     += len;
        m_file_chunk.remaind -= len;
        return true;
    }
    // требуемый размер больше чем остаток в буфере, считаем данные из файла
    // если есть непрочитанный остаток, сместим его в начало буфера 
    if (m_file_chunk.remaind)
    {
        uint8_t *src, *dst;

        src = m_file_chunk.data + m_file_chunk.pos;
        dst = m_file_chunk.data;

        memmove(dst, src, m_file_chunk.remaind);
        m_file_chunk.size = m_file_chunk.remaind;
    }
    else
        m_file_chunk.size = 0;



    uint32_t amout;
    // вычислим размер данных который нужно прочитать из файла 
    if (m_file_chunk.file_remaind < FILE_CHUNK)
        amout = (uint32_t)m_file_chunk.file_remaind;
    else
        amout = FILE_CHUNK;

    // переаллоцируем размер чанка 
    if (!m_file_chunk.Resize(amout + m_file_chunk.remaind))
        return false;

    // читаем данные из файла 
    if (!FileRead(m_file_chunk.data + m_file_chunk.remaind, amout))
        return false;

    // скорректируем смещения
    m_file_chunk.file_remaind -= amout;
    m_file_chunk.remaind      += amout;
    m_file_chunk.pos           = 0;

    // отдадим данные  
    if (len <= m_file_chunk.remaind)
    {
        *data  =  m_file_chunk.data;

        m_file_chunk.pos     += len;
        m_file_chunk.remaind -= len;
        return true;
    }

    // требуемый размер больше чем осталось в буфере, выходим с ошибкой 
    return false;
}

/*
*/
bool BufferedFile::IsEOF()
{
    bool r = m_file_chunk.remaind == 0 && m_file_chunk.file_remaind == 0;
    return r;
}

uint32_t BufferedFile::GetRemaind() const
{
    uint32_t remaind = (uint32_t)m_file_chunk.file_remaind + m_file_chunk.remaind;
    return remaind;
}

void BufferedFile::Shutdown()
{
    FileClose();
    m_file_chunk.Free();
    memset(&m_file_chunk, 0, sizeof(m_file_chunk));
}

bool BufferedFile::FileClose()
{
    if (m_file != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_file);
        m_file = INVALID_HANDLE_VALUE;
    }
    return true;
}

bool BufferedFile::FileRead(uint8_t *data, uint32_t len)
{
    BOOL  r;
    DWORD readed = 0;

    r = ReadFile(m_file, data, len, &readed, NULL);
    if (r == FALSE)
        return false;

    if (readed != len)
        return false;

    return true;
}

uint64_t BufferedFile::FileSize()
{
    uint64_t size;
    DWORD    high_size = 0;
    DWORD    low_size;

    low_size = GetFileSize(m_file, &high_size);

    if (high_size)
    {
        size = (((uint64_t)high_size << 32) & 0xFFFFFFFF00000000) | (((uint64_t)low_size) & 0xFFFFFFFF);
    }
    else
    {
        size = (uint64_t)low_size;
    }

    return size;
}