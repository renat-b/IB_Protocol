#include "stdint.h"

#pragma pack(push, 1)
typedef struct _ToolAddress
{
    uint8_t      tool_id;
    uint16_t     serial_num;
} ToolAddress;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct _IndigoBaseTransportHeader
{
    uint8_t      version;
    uint8_t      header_length;
    ToolAddress  address_source;
    ToolAddress  address_destination;
    uint16_t     frame_num;
    uint8_t      flags;
    uint8_t      crc8_header;

    uint16_t     data_length;
    uint16_t     data_offset;
    uint16_t     data_crc;
} IndigoBaseTransportHeader;
#pragma pack(pop)