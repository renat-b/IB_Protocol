#pragma once
//+------------------------------------------------------------------+
//| Базовый класс-обертка вокруг WinAPI (файлы не больше 4Gb)        |
//+------------------------------------------------------------------+
class BinFile
{
public:
   static const UINT64 INVALID_POSITION;

protected:
   HANDLE            m_file;                 // хендл файла

public:
   BinFile() : m_file(INVALID_HANDLE_VALUE) {}
   ~BinFile()                               { Close(); }
   //---
   inline bool       Open(const char *lpFileName,const DWORD dwAccess,const DWORD dwShare,const DWORD dwCreationFlags,const DWORD dwAttributes=FILE_ATTRIBUTE_NORMAL);
   //---
   inline bool       OpenRead(const char *lpFileName)  { return Open(lpFileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,OPEN_EXISTING);  }
   inline bool       OpenWrite(const char *lpFileName) { return Open(lpFileName,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,CREATE_ALWAYS); }
   //---
   inline void       Close(void);
   inline UINT64     Size(void) const;
   static UINT64     Size(const char *path);
   inline FILETIME   Time(void) const;
   inline HANDLE     Handle(void)   { return(m_file);                       }
   inline bool       IsOpen() const { return(m_file!=INVALID_HANDLE_VALUE); }
   inline DWORD      Read(void  *buffer,const DWORD length);
   inline DWORD      Write(const void *buffer,const DWORD length);
   inline UINT64     Seek(const INT64 distance,const DWORD method);
   inline bool       Flush();
   inline bool       ChangeSize(const UINT64 size);
   inline UINT64     CurrPos();
   char*             Load(const char *filename,DWORD &size);
   inline BOOL       SetFileTime(const FILETIME *create,const FILETIME *access,const FILETIME *write);
};

const __declspec(selectany) UINT64 BinFile::INVALID_POSITION=_UI64_MAX;
//+------------------------------------------------------------------+
//| Открытие файла для чтения                                        |
//+------------------------------------------------------------------+
inline bool BinFile::Open(const char *lpFileName,const DWORD dwAccess,const DWORD dwShare,const DWORD dwCreationFlags,const DWORD dwAttributes)
{
    Close();

    if(lpFileName)
        m_file=CreateFile(lpFileName,dwAccess,dwShare,NULL,dwCreationFlags,dwAttributes,NULL);

    return(m_file!=INVALID_HANDLE_VALUE);
}
//+------------------------------------------------------------------+
//| Закрытие файла                                                   |
//+------------------------------------------------------------------+
inline void BinFile::Close()
{
    if(m_file != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_file);
        m_file=INVALID_HANDLE_VALUE;
    }
}
//+------------------------------------------------------------------+
//| Размер файла                                                     |
//+------------------------------------------------------------------+
inline UINT64 BinFile::Size() const
{
    LARGE_INTEGER li={0};

    if(m_file == INVALID_HANDLE_VALUE) 
        return(0);

    if((li.LowPart = ::GetFileSize(m_file,(LPDWORD)&li.HighPart)) == INVALID_FILE_SIZE)
        if(GetLastError() != NO_ERROR) 
            return(0);

    return((UINT64)li.QuadPart);
}
//+------------------------------------------------------------------+
//| Размер файла                                                     |
//+------------------------------------------------------------------+
inline UINT64 BinFile::Size(const char *path)
{
    LARGE_INTEGER             li={0};
    WIN32_FILE_ATTRIBUTE_DATA fad;

    if(GetFileAttributesEx(path, GetFileExInfoStandard, &fad))
    {
        li.LowPart =fad.nFileSizeLow;
        li.HighPart=(LONG)fad.nFileSizeHigh;
    }
    return((UINT64)li.QuadPart);
}
//+------------------------------------------------------------------+
//| Размер файла                                                     |
//+------------------------------------------------------------------+
inline FILETIME BinFile::Time() const
{
    FILETIME ft={0};

    if(m_file != INVALID_HANDLE_VALUE) 
        GetFileTime(m_file,NULL,NULL,&ft);

    return(ft);
}
//+------------------------------------------------------------------+
//| Запись буфера указанной длины в файл                             |
//+------------------------------------------------------------------+
inline DWORD BinFile::Read(void *buffer,const DWORD length)
{
    DWORD readed=0;

    if(m_file == INVALID_HANDLE_VALUE || buffer == NULL || length < 1) 
        return(0);

    if(ReadFile(m_file, buffer, length, &readed, NULL) == 0) 
        readed = 0;

    return(readed);
}
//+------------------------------------------------------------------+
//| Запись буфера указанной длины в файл                             |
//+------------------------------------------------------------------+
inline DWORD BinFile::Write(const void *buffer,const DWORD length)
{
    DWORD written=0;

    if(m_file != INVALID_HANDLE_VALUE && buffer && length>0)
    {
        if(WriteFile(m_file, buffer, length, &written, NULL) == 0)
            written = 0;
    }
    return(written);
}
//+------------------------------------------------------------------+
//| Перемещение файлового указателЯ                                  |
//+------------------------------------------------------------------+
inline UINT64 BinFile::Seek(const INT64 distance,const DWORD method)
{
    LARGE_INTEGER li={0};

    if(m_file == INVALID_HANDLE_VALUE) 
        return(INVALID_POSITION);

    li.QuadPart = distance;
    li.LowPart  = SetFilePointer(m_file,(LONG)li.LowPart,&li.HighPart,method);

    if(li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) 
        return(INVALID_POSITION);

    return((UINT64)li.QuadPart);
}
//+------------------------------------------------------------------+
//| Сброс файла на диск                                              |
//+------------------------------------------------------------------+
inline bool BinFile::Flush()
{
    if (m_file != INVALID_HANDLE_VALUE) 
        return(::FlushFileBuffers(m_file)!=0);
    return(false);
}
//+------------------------------------------------------------------+
//| Изменение размера файла                                          |
//+------------------------------------------------------------------+
inline bool BinFile::ChangeSize(const UINT64 size)
{
    return(BinFile::Seek((INT64)size, FILE_BEGIN) == size && SetEndOfFile(m_file));
}
//+------------------------------------------------------------------+
//| Текущее положение файлового указателЯ                            |
//+------------------------------------------------------------------+
inline UINT64 BinFile::CurrPos()
{
    return(BinFile::Seek(INT64(0), FILE_CURRENT));
}
//+------------------------------------------------------------------+
//| Чтение в выделенный буфер всего файла                            |
//+------------------------------------------------------------------+
inline char* BinFile::Load(const char *filename,DWORD &size)
{
    DWORD  readed=0;
    char  *buffer;
//--- проверки
    if(filename==NULL)            return(NULL);
    if(!OpenRead(filename))       return(NULL);
    if((size=(DWORD)Size()) == 0)
    {
        Close();
        return(NULL);
    }
//--- выделим буфер
    if((buffer = new(std::nothrow) char[size+16])==NULL)
    {
       Close();
       return(NULL);
    }
//--- считаем и вернем результат
    if(size>0)
        if(ReadFile(m_file,buffer,size,&readed,NULL)==0)
        {
            //--- освободим буфер
            delete[] buffer;
            buffer=NULL;
        }
//--- закроем файл и вернем ссылку на буфер
    Close();
    return(buffer);
}
//+------------------------------------------------------------------+
//| Установка атрибутов времени файла                                |
//+------------------------------------------------------------------+
inline BOOL BinFile::SetFileTime(const FILETIME *create,const FILETIME *access,const FILETIME *write)
{
//--- проверки
    if(m_file == INVALID_HANDLE_VALUE) 
        return(FALSE);
//--- устанавливаем время
    return(::SetFileTime(m_file, create, access, write));
}