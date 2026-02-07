#pragma once

#include <stdint.h>

typedef enum {
    SomfyRtsCmdStop = 0x1,
    SomfyRtsCmdOpen = 0x2,
    SomfyRtsCmdClose = 0x4,
    SomfyRtsCmdProg = 0x8,
} SomfyRtsCmd;

void somfy_rts_send(uint32_t address, uint16_t rolling_code, SomfyRtsCmd cmd);
