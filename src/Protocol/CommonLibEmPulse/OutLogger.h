#pragma once

enum LoggerCode
{
    LogCmdError = 0,
    LogCmdWarning,
    LogCmdOK,
    LogCmdDebug,
};

class OutLogger
{
public:
    virtual bool    Out(int cmd, const char *source, const char *fmt, ...) = 0;
};

class LoggerPrint
{
protected:
    typedef void(*FuncPrintLog)(uint32_t code, const char *source, const char *msg);


protected:
    FuncPrintLog      m_print_log_func;
    char              m_print_log_source[32];

public:
    LoggerPrint();
    virtual  ~LoggerPrint();

    void       SetPrintLog(FuncPrintLog print_func, const char *source);

protected:
    void       PrintLog(uint32_t code, const char *format, ...);
};

