#include "somfy_rts_store.h"

#include <furi.h>
#include <storage/storage.h>
#include <flipper_format/flipper_format.h>

#define SOMFY_RTS_CONFIG_PATH    APP_DATA_PATH("config.txt")
#define SOMFY_RTS_CONFIG_FILETYPE "Somfy RTS Config"
#define SOMFY_RTS_CONFIG_VERSION  1

bool somfy_rts_store_load(SomfyRtsApp* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_file_alloc(storage);
    bool success = false;

    app->blind_count = 0;

    do {
        if(!flipper_format_file_open_existing(ff, SOMFY_RTS_CONFIG_PATH)) break;

        FuriString* filetype = furi_string_alloc();
        uint32_t version = 0;
        if(!flipper_format_read_header(ff, filetype, &version)) {
            furi_string_free(filetype);
            break;
        }
        if(furi_string_cmp_str(filetype, SOMFY_RTS_CONFIG_FILETYPE) != 0 ||
           version != SOMFY_RTS_CONFIG_VERSION) {
            furi_string_free(filetype);
            break;
        }
        furi_string_free(filetype);

        FuriString* name = furi_string_alloc();
        while(app->blind_count < SOMFY_RTS_MAX_BLINDS) {
            if(!flipper_format_read_string(ff, "Blind", name)) break;

            uint32_t address = 0;
            uint32_t rolling_code = 0;
            if(!flipper_format_read_uint32(ff, "Address", &address, 1)) break;
            if(!flipper_format_read_uint32(ff, "Rolling_Code", &rolling_code, 1)) break;

            SomfyBlind* blind = &app->blinds[app->blind_count];
            strncpy(blind->name, furi_string_get_cstr(name), SOMFY_RTS_NAME_MAX - 1);
            blind->name[SOMFY_RTS_NAME_MAX - 1] = '\0';
            blind->address = address & 0xFFFFFF;
            blind->rolling_code = (uint16_t)rolling_code;
            app->blind_count++;
        }
        furi_string_free(name);
        success = true;
    } while(false);

    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);
    return success;
}

bool somfy_rts_store_save(SomfyRtsApp* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_file_alloc(storage);
    bool success = false;

    do {
        if(!storage_simply_mkdir(storage, APP_DATA_PATH(""))) {
            // Directory may already exist, continue
        }

        if(!flipper_format_file_open_always(ff, SOMFY_RTS_CONFIG_PATH)) break;
        if(!flipper_format_write_header_cstr(ff, SOMFY_RTS_CONFIG_FILETYPE, SOMFY_RTS_CONFIG_VERSION))
            break;

        bool write_ok = true;
        for(uint8_t i = 0; i < app->blind_count; i++) {
            SomfyBlind* blind = &app->blinds[i];
            uint32_t address = blind->address;
            uint32_t rolling_code = blind->rolling_code;

            if(!flipper_format_write_string_cstr(ff, "Blind", blind->name)) {
                write_ok = false;
                break;
            }
            if(!flipper_format_write_uint32(ff, "Address", &address, 1)) {
                write_ok = false;
                break;
            }
            if(!flipper_format_write_uint32(ff, "Rolling_Code", &rolling_code, 1)) {
                write_ok = false;
                break;
            }
        }
        if(!write_ok) break;
        success = true;
    } while(false);

    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);
    return success;
}
