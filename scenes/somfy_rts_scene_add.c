#include "somfy_rts_scene.h"
#include "../somfy_rts_store.h"
#include <furi_hal_random.h>

static void somfy_rts_scene_add_text_input_cb(void* context) {
    SomfyRtsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, SomfyRtsCustomEventTextInputDone);
}

void somfy_rts_scene_add_on_enter(void* context) {
    SomfyRtsApp* app = context;
    text_input_reset(app->text_input);
    text_input_set_header_text(app->text_input, "Blind Name:");
    text_input_set_minimum_length(app->text_input, 1);
    app->text_buf[0] = '\0';
    text_input_set_result_callback(
        app->text_input,
        somfy_rts_scene_add_text_input_cb,
        app,
        app->text_buf,
        SOMFY_RTS_NAME_MAX,
        true);
    view_dispatcher_switch_to_view(app->view_dispatcher, SomfyRtsViewTextInput);
}

bool somfy_rts_scene_add_on_event(void* context, SceneManagerEvent event) {
    SomfyRtsApp* app = context;
    if(event.type == SceneManagerEventTypeCustom &&
       event.event == SomfyRtsCustomEventTextInputDone) {
        if(app->blind_count < SOMFY_RTS_MAX_BLINDS && strlen(app->text_buf) > 0) {
            SomfyBlind* blind = &app->blinds[app->blind_count];
            strncpy(blind->name, app->text_buf, SOMFY_RTS_NAME_MAX - 1);
            blind->name[SOMFY_RTS_NAME_MAX - 1] = '\0';
            blind->address = furi_hal_random_get() & 0xFFFFFF;
            blind->rolling_code = 1;
            app->blind_count++;
            somfy_rts_store_save(app);
        }
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }
    return false;
}

void somfy_rts_scene_add_on_exit(void* context) {
    SomfyRtsApp* app = context;
    text_input_reset(app->text_input);
}
