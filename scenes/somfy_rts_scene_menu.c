#include "somfy_rts_scene.h"
#include <input/input.h>

static void somfy_rts_scene_menu_callback_ex(void* context, InputType input_type, uint32_t index) {
    SomfyRtsApp* app = context;
    if(input_type != InputTypeShort && input_type != InputTypeLong) return;

    if(index == SOMFY_RTS_ALL_BLINDS) {
        if(input_type == InputTypeShort) {
            app->selected_blind = SOMFY_RTS_ALL_BLINDS;
            view_dispatcher_send_custom_event(
                app->view_dispatcher, SomfyRtsCustomEventMenuSelected);
        }
    } else if(index < app->blind_count) {
        app->selected_blind = index;
        if(input_type == InputTypeLong) {
            view_dispatcher_send_custom_event(
                app->view_dispatcher, SomfyRtsCustomEventDeleteBlind);
        } else {
            view_dispatcher_send_custom_event(
                app->view_dispatcher, SomfyRtsCustomEventMenuSelected);
        }
    } else if(input_type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, SomfyRtsCustomEventAddBlind);
    }
}

void somfy_rts_scene_menu_on_enter(void* context) {
    SomfyRtsApp* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Somfy RTS");

    if(app->blind_count >= 2) {
        submenu_add_item_ex(
            app->submenu, "All Blinds", SOMFY_RTS_ALL_BLINDS,
            somfy_rts_scene_menu_callback_ex, app);
    }

    for(uint8_t i = 0; i < app->blind_count; i++) {
        submenu_add_item_ex(
            app->submenu, app->blinds[i].name, i, somfy_rts_scene_menu_callback_ex, app);
    }
    submenu_add_item_ex(
        app->submenu, "Add Blind", app->blind_count, somfy_rts_scene_menu_callback_ex, app);

    uint32_t state = scene_manager_get_scene_state(app->scene_manager, SomfyRtsSceneMenu);
    submenu_set_selected_item(app->submenu, state);

    view_dispatcher_switch_to_view(app->view_dispatcher, SomfyRtsViewMenu);
}

bool somfy_rts_scene_menu_on_event(void* context, SceneManagerEvent event) {
    SomfyRtsApp* app = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SomfyRtsCustomEventMenuSelected) {
            scene_manager_set_scene_state(
                app->scene_manager, SomfyRtsSceneMenu, app->selected_blind);
            scene_manager_next_scene(app->scene_manager, SomfyRtsSceneControl);
            return true;
        } else if(event.event == SomfyRtsCustomEventAddBlind) {
            scene_manager_next_scene(app->scene_manager, SomfyRtsSceneAdd);
            return true;
        } else if(event.event == SomfyRtsCustomEventDeleteBlind) {
            scene_manager_next_scene(app->scene_manager, SomfyRtsSceneConfirmDelete);
            return true;
        }
    }
    return false;
}

void somfy_rts_scene_menu_on_exit(void* context) {
    SomfyRtsApp* app = context;
    submenu_reset(app->submenu);
}
