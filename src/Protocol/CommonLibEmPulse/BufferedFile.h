#pragma once

#include "windows.h"
#include "stdint.h"
#include "new.h"

class BufferedFile
{
private:
    enum constants
    {
        Kb         = 1024,
        Mb         = Kb * Kb,
        FILE_CHUNK = 16 * Mb,
    };
    // внутренний буфера чтения из файла  
    struct FileChunk
    {
        uint8_t    *data     = nullptr;
        uint32_t    size     = 0;
        uint32_t    max_size = 0;

        uint32_t    pos      = 0;
        uint32_t    remaind  = 0;
        uint64_t    file_remaind = 0;

        bool        Resize(uint32_t new_len);
        void        Free();
    };


private:
    // буфер чтения    
    FileChunk          m_file_chunk;
    //  хендл файла 
    HANDLE             m_file = INVALID_HANDLE_VALUE;


public:
    BufferedFile();
    ~BufferedFile();

    // чтение внутреннего буфера
    bool            FileOpen(const char *file_name);
    bool            Next(uint8_t **data, uint32_t len);
    bool            IsEOF();

    uint32_t        GetRemaind() const;

private:
    void            Shutdown();

    bool            FileClose();
    bool            FileRead(uint8_t *data, uint32_t len);
    uint64_t        FileSize();
};