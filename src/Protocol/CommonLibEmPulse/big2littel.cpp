#include "StdAfx.h"
#include "big2littel.h"


void Big2LittelEndian(void *data, uint32_t size)
{
    uint8_t *dst, *src;
    uint8_t  tmp;
    
    dst = (uint8_t *)data;
    src = (uint8_t *)data + size - 1;
    while (dst < src)
    {
        tmp  = *dst;
        *dst = *src;
        *src = tmp;

        dst++; 
        src--;
    }
}