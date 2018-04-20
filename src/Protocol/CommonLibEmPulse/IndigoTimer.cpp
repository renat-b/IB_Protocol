#include "IndigoTimer.h"

LARGE_INTEGER IndigoTimer::s_frequency = { 0 };


IndigoTimer::IndigoTimer() : m_work(false), m_elapsed(0), m_delay(0)
{
    m_start_time.QuadPart = 0;
}


void IndigoTimer::Initialize()
{
    m_elapsed = 0;
    if (!s_frequency.QuadPart)
        QueryPerformanceFrequency(&s_frequency);
}


void IndigoTimer::Wait(DWORD delay_ms)
{
    LARGE_INTEGER start_time, end_time, elapsed;

    // переводим в микросекунды
    delay_ms *= MILLISECOND_IN_SECOND;
    // получаем начальное время
    QueryPerformanceCounter(&start_time);
    do
    {
        QueryPerformanceCounter(&end_time);
        // получаем разницу в тиках
        elapsed.QuadPart = end_time.QuadPart - start_time.QuadPart;
        // переводим в милисекунды 
        elapsed.QuadPart *= MICROSECOND_IN_SECOND;
        elapsed.QuadPart /= s_frequency.QuadPart;
    } 
    while (elapsed.QuadPart < delay_ms);
}


void IndigoTimer::Start(DWORD delay_ms)
{
    m_work = true;
    // запоминаем данные
    // переводим в микросекунды
    m_delay   = delay_ms * MILLISECOND_IN_SECOND;
    m_elapsed = 0;

    // инициализация
    Initialize();
    // получим начальное (стартовое) значение в тактах
    QueryPerformanceCounter(&m_start_time);
}


uint64_t IndigoTimer::Stop()
{
    LARGE_INTEGER   elapsed;
    LARGE_INTEGER   end_time;
    // получим текущие такты
    QueryPerformanceCounter(&end_time);

    // пересчитаем из тактов в ms
    elapsed.QuadPart  = end_time.QuadPart - m_start_time.QuadPart;
    elapsed.QuadPart *= MICROSECOND_IN_SECOND;
    elapsed.QuadPart /= s_frequency.QuadPart;
    
    // вернем результат в микросекундах 
    return (elapsed.QuadPart / MILLISECOND_IN_SECOND);
}


bool IndigoTimer::IsExpired()
{
    LARGE_INTEGER   elapsed;
    LARGE_INTEGER   end_time;
    // получим текущие такты
    QueryPerformanceCounter(&end_time);

    // пересчитаем из тактов в ms
    elapsed.QuadPart  = end_time.QuadPart - m_start_time.QuadPart;
    elapsed.QuadPart *= MICROSECOND_IN_SECOND;
    elapsed.QuadPart /= s_frequency.QuadPart;
    
    // проверим, истек срок?
    m_elapsed = elapsed.QuadPart; 
    if (m_elapsed > m_delay)
        return true;

    return false;
}


uint64_t IndigoTimer::ElapsedTimeMs()
{
    uint64_t ms;

    ms = m_elapsed;
    ms /= MILLISECOND_IN_SECOND;
    return ms;
}


bool IndigoTimer::IsWork() const
{
    return m_work;
}
