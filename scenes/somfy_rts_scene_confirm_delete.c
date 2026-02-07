#include "somfy_rts_scene.h"
#include "../somfy_rts_store.h"

static void somfy_rts_scene_confirm_delete_cb(DialogExResult result, void* context) {
    SomfyRtsApp* app = context;
    if(result == DialogExResultRight) {
        view_dispatcher_send_custom_event(app->view_dispatcher, SomfyRtsCustomEventDeleteConfirm);
    } else {
        view_dispatcher_send_custom_event(app->view_dispatcher, SomfyRtsCustomEventDeleteCancel);
    }
}

void somfy_rts_scene_confirm_delete_on_enter(void* context) {
    SomfyRtsApp* app = context;
    SomfyBlind* blind = &app->blinds[app->selected_blind];

    dialog_ex_set_header(app->dialog_ex, "Delete Blind?", 64, 0, AlignCenter, AlignTop);
    dialog_ex_set_text(app->dialog_ex, blind->name, 64, 26, AlignCenter, AlignCenter);
    dialog_ex_set_left_button_text(app->dialog_ex, "Cancel");
    dialog_ex_set_right_button_text(app->dialog_ex, "Delete");
    dialog_ex_set_result_callback(app->dialog_ex, somfy_rts_scene_confirm_delete_cb);
    dialog_ex_set_context(app->dialog_ex, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SomfyRtsViewConfirmDelete);
}

bool somfy_rts_scene_confirm_delete_on_event(void* context, SceneManagerEvent event) {
    SomfyRtsApp* app = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SomfyRtsCustomEventDeleteConfirm) {
            // Remove blind by shifting remaining entries
            for(uint8_t i = app->selected_blind; i < app->blind_count - 1; i++) {
                app->blinds[i] = app->blinds[i + 1];
            }
            app->blind_count--;
            somfy_rts_store_save(app);
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, SomfyRtsSceneMenu);
            return true;
        } else if(event.event == SomfyRtsCustomEventDeleteCancel) {
            scene_manager_previous_scene(app->scene_manager);
            return true;
        }
    }
    return false;
}

void somfy_rts_scene_confirm_delete_on_exit(void* context) {
    SomfyRtsApp* app = context;
    dialog_ex_reset(app->dialog_ex);
}
