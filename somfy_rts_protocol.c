#include "somfy_rts_protocol.h"

#include <furi.h>
#include <furi_hal.h>
#include <lib/subghz/devices/devices.h>
#include <lib/subghz/devices/cc1101_int/cc1101_int_interconnect.h>

#define SOMFY_RTS_FREQ        433420000
#define SOMFY_RTS_SYMBOL_US   640
#define SOMFY_RTS_KEY         0xA7

// Timing constants (microseconds)
#define SOMFY_RTS_WAKEUP_HIGH  10568
#define SOMFY_RTS_WAKEUP_LOW   7072
#define SOMFY_RTS_PREAMBLE_HIGH 2585
#define SOMFY_RTS_PREAMBLE_LOW  2436
#define SOMFY_RTS_SYNC_HIGH    4898
#define SOMFY_RTS_SYNC_LOW     644
#define SOMFY_RTS_INTER_FRAME  26838

typedef enum {
    TxStateWakeupHigh,
    TxStateWakeupLow,
    TxStatePreamble,
    TxStateSyncHigh,
    TxStateSyncLow,
    TxStateData,
    TxStateInterFrame,
    TxStateDone,
} TxState;

typedef struct {
    uint8_t frame[7];
    TxState state;
    uint8_t repeat;       // which repetition (0 = first with wakeup, 1-2 = repeats)
    uint8_t preamble_idx; // preamble pulse counter
    uint8_t bit_idx;      // current bit index in frame (0..55)
    bool manchester_half; // false = first half of manchester symbol, true = second half
} SomfyRtsTxState;

static void somfy_rts_build_frame(
    uint8_t* frame,
    uint32_t address,
    uint16_t rolling_code,
    SomfyRtsCmd cmd) {
    // Byte 0: Key (encryption key, typically 0xA7)
    frame[0] = SOMFY_RTS_KEY;
    // Byte 1: Command (high nibble) | Checksum placeholder (low nibble)
    frame[1] = (uint8_t)cmd << 4;
    // Bytes 2-3: Rolling code (big-endian)
    frame[2] = (rolling_code >> 8) & 0xFF;
    frame[3] = rolling_code & 0xFF;
    // Bytes 4-6: Address (little-endian)
    frame[4] = address & 0xFF;
    frame[5] = (address >> 8) & 0xFF;
    frame[6] = (address >> 16) & 0xFF;

    // Calculate checksum: XOR of all nibbles
    uint8_t checksum = 0;
    for(int i = 0; i < 7; i++) {
        checksum ^= (frame[i] >> 4) ^ (frame[i] & 0x0F);
    }
    checksum &= 0x0F;
    frame[1] |= checksum;

    // Obfuscation: forward rolling XOR
    for(int i = 1; i < 7; i++) {
        frame[i] ^= frame[i - 1];
    }
}

static LevelDuration somfy_rts_tx_callback(void* context) {
    SomfyRtsTxState* tx = context;

    switch(tx->state) {
    case TxStateWakeupHigh:
        tx->state = TxStateWakeupLow;
        return level_duration_make(true, SOMFY_RTS_WAKEUP_HIGH);

    case TxStateWakeupLow:
        tx->state = TxStatePreamble;
        tx->preamble_idx = 0;
        return level_duration_make(false, SOMFY_RTS_WAKEUP_LOW);

    case TxStatePreamble: {
        uint8_t max_pulses = (tx->repeat == 0) ? 4 : 14; // 2 or 7 preamble cycles (H+L each)
        bool level = (tx->preamble_idx % 2 == 0); // even=high, odd=low
        uint32_t duration = level ? SOMFY_RTS_PREAMBLE_HIGH : SOMFY_RTS_PREAMBLE_LOW;
        tx->preamble_idx++;
        if(tx->preamble_idx >= max_pulses) {
            tx->state = TxStateSyncHigh;
        }
        return level_duration_make(level, duration);
    }

    case TxStateSyncHigh:
        tx->state = TxStateSyncLow;
        return level_duration_make(true, SOMFY_RTS_SYNC_HIGH);

    case TxStateSyncLow:
        tx->state = TxStateData;
        tx->bit_idx = 0;
        tx->manchester_half = false;
        return level_duration_make(false, SOMFY_RTS_SYNC_LOW);

    case TxStateData: {
        // Manchester encode 56 bits (7 bytes), MSB first
        uint8_t byte_idx = tx->bit_idx / 8;
        uint8_t bit_pos = 7 - (tx->bit_idx % 8); // MSB first
        bool bit_val = (tx->frame[byte_idx] >> bit_pos) & 1;

        bool level;
        if(!tx->manchester_half) {
            // First half: bit=1 -> high, bit=0 -> low
            level = bit_val;
            tx->manchester_half = true;
        } else {
            // Second half: inverted
            level = !bit_val;
            tx->manchester_half = false;
            tx->bit_idx++;
            if(tx->bit_idx >= 56) {
                tx->state = TxStateInterFrame;
            }
        }
        return level_duration_make(level, SOMFY_RTS_SYMBOL_US);
    }

    case TxStateInterFrame:
        tx->repeat++;
        if(tx->repeat >= 3) {
            tx->state = TxStateDone;
        } else {
            // Repeats skip wakeup, go straight to preamble
            tx->state = TxStatePreamble;
            tx->preamble_idx = 0;
        }
        return level_duration_make(false, SOMFY_RTS_INTER_FRAME);

    case TxStateDone:
    default:
        return level_duration_reset();
    }
}

void somfy_rts_send(uint32_t address, uint16_t rolling_code, SomfyRtsCmd cmd) {
    SomfyRtsTxState tx;
    memset(&tx, 0, sizeof(tx));
    somfy_rts_build_frame(tx.frame, address, rolling_code, cmd);
    tx.state = TxStateWakeupHigh;
    tx.repeat = 0;

    subghz_devices_init();
    const SubGhzDevice* device = subghz_devices_get_by_name(SUBGHZ_DEVICE_CC1101_INT_NAME);
    subghz_devices_begin(device);
    subghz_devices_reset(device);
    subghz_devices_load_preset(device, FuriHalSubGhzPresetOok270Async, NULL);
    subghz_devices_set_frequency(device, SOMFY_RTS_FREQ);

    subghz_devices_set_tx(device);
    subghz_devices_start_async_tx(device, somfy_rts_tx_callback, &tx);

    while(!subghz_devices_is_async_complete_tx(device)) {
        furi_delay_ms(10);
    }

    subghz_devices_stop_async_tx(device);
    subghz_devices_idle(device);
    subghz_devices_end(device);
    subghz_devices_deinit();
}
