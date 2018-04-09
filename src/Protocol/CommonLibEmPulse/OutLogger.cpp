#include "StdAfx.h"
#include "OutLogger.h"


LoggerPrint::LoggerPrint() : m_print_log_func(NULL)
{
    memset(m_print_log_source, 0, sizeof(m_print_log_source));
}

LoggerPrint::~LoggerPrint()
{

}

void LoggerPrint::SetPrintLog(FuncPrintLog print_func, const char *source)
{
    if (!source || !print_func)
        return;

    m_print_log_func = print_func;
    strcpy_s(m_print_log_source, source);
}

void LoggerPrint::PrintLog(uint32_t code, const char *fmt, ...)
{
    if (!m_print_log_func)
        return;

    char    buf[1024];
    va_list arg_ptr;
    int     len;

    //--- форматируем строку
    va_start(arg_ptr, fmt);
    len = _vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, arg_ptr);

    if (len <= -1)
        return;
    va_end(arg_ptr);

    m_print_log_func(code, m_print_log_source, buf);
}