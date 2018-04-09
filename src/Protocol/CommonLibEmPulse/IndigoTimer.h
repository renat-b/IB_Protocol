#pragma once

#include "windows.h"
#include "stdint.h"

class IndigoTimer
{
private:
    enum constants
    {
        MICROSECOND_IN_SECOND = 1000000,
        MILLISECOND_IN_SECOND = 1000,
    };


private:
    static LARGE_INTEGER s_frequency;      // частота таймера 


private:
    LARGE_INTEGER     m_start_time;        // начальный отсчет в тактах
    uint64_t          m_elapsed;           // текущий интервал
    uint64_t          m_delay;             // общее время ожидания в мс
    bool              m_work;

public:
    IndigoTimer();

    void              Initialize();

    void              Wait(DWORD delay_ms);

    void              Start(DWORD delay_ms);
    uint64_t          Stop();
    bool              IsExpired();

    uint64_t          ElapsedTimeMs();
    bool              IsWork() const;
};