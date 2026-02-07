#include "somfy_rts_scene.h"

static void somfy_rts_scene_menu_callback(void* context, uint32_t index) {
    SomfyRtsApp* app = context;
    if(index < app->blind_count) {
        app->selected_blind = index;
        view_dispatcher_send_custom_event(app->view_dispatcher, SomfyRtsCustomEventMenuSelected);
    } else {
        view_dispatcher_send_custom_event(app->view_dispatcher, SomfyRtsCustomEventAddBlind);
    }
}

void somfy_rts_scene_menu_on_enter(void* context) {
    SomfyRtsApp* app = context;
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Somfy RTS");

    for(uint8_t i = 0; i < app->blind_count; i++) {
        submenu_add_item(app->submenu, app->blinds[i].name, i, somfy_rts_scene_menu_callback, app);
    }
    submenu_add_item(
        app->submenu, "Add Blind", app->blind_count, somfy_rts_scene_menu_callback, app);

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
        }
    }
    return false;
}

void somfy_rts_scene_menu_on_exit(void* context) {
    SomfyRtsApp* app = context;
    submenu_reset(app->submenu);
}
