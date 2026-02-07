#include "somfy_rts_scene.h"
#include "../somfy_rts_protocol.h"
#include "../somfy_rts_store.h"
#include <gui/canvas.h>

static void somfy_rts_scene_control_send(SomfyRtsApp* app, SomfyRtsCmd cmd) {
    if(app->selected_blind == SOMFY_RTS_ALL_BLINDS) {
        for(uint8_t i = 0; i < app->blind_count; i++) {
            SomfyBlind* blind = &app->blinds[i];
            somfy_rts_send(blind->address, blind->rolling_code, cmd);
            blind->rolling_code++;
        }
    } else {
        SomfyBlind* blind = &app->blinds[app->selected_blind];
        somfy_rts_send(blind->address, blind->rolling_code, cmd);
        blind->rolling_code++;
    }
    somfy_rts_store_save(app);
}

static void somfy_rts_scene_control_draw(Canvas* canvas, void* model) {
    SomfyRtsApp* app = *(SomfyRtsApp**)model;
    bool all_mode = (app->selected_blind == SOMFY_RTS_ALL_BLINDS);

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);

    if(all_mode) {
        canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "All Blinds");
        char buf[32];
        canvas_set_font(canvas, FontSecondary);
        snprintf(buf, sizeof(buf), "%u blinds configured", app->blind_count);
        canvas_draw_str_aligned(canvas, 64, 16, AlignCenter, AlignTop, buf);
    } else {
        SomfyBlind* blind = &app->blinds[app->selected_blind];
        canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, blind->name);
        char buf[40];
        canvas_set_font(canvas, FontSecondary);
        snprintf(buf, sizeof(buf), "Addr: %06lX  RC: %u", blind->address, blind->rolling_code);
        canvas_draw_str_aligned(canvas, 64, 16, AlignCenter, AlignTop, buf);
    }

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 30, AlignCenter, AlignTop, "[UP] Open");
    canvas_draw_str_aligned(canvas, 64, 40, AlignCenter, AlignTop, "[OK] Stop  [DOWN] Close");
    if(!all_mode) {
        canvas_draw_str_aligned(canvas, 64, 50, AlignCenter, AlignTop, "Hold OK = Prog");
    }
}

static bool somfy_rts_scene_control_input(InputEvent* event, void* context) {
    SomfyRtsApp* app = context;

    if(event->key == InputKeyBack) {
        return false;
    }

    if(event->type == InputTypeShort) {
        switch(event->key) {
        case InputKeyUp:
            somfy_rts_scene_control_send(app, SomfyRtsCmdOpen);
            view_commit_model(app->control_view, true);
            return true;
        case InputKeyDown:
            somfy_rts_scene_control_send(app, SomfyRtsCmdClose);
            view_commit_model(app->control_view, true);
            return true;
        case InputKeyOk:
            somfy_rts_scene_control_send(app, SomfyRtsCmdStop);
            view_commit_model(app->control_view, true);
            return true;
        default:
            break;
        }
    } else if(event->type == InputTypeLong && event->key == InputKeyOk) {
        // Prog only for individual blinds
        if(app->selected_blind != SOMFY_RTS_ALL_BLINDS) {
            somfy_rts_scene_control_send(app, SomfyRtsCmdProg);
            view_commit_model(app->control_view, true);
        }
        return true;
    }

    return false;
}

void somfy_rts_scene_control_on_enter(void* context) {
    SomfyRtsApp* app = context;
    view_set_draw_callback(app->control_view, somfy_rts_scene_control_draw);
    view_set_input_callback(app->control_view, somfy_rts_scene_control_input);
    view_set_context(app->control_view, app);
    view_commit_model(app->control_view, true);
    view_dispatcher_switch_to_view(app->view_dispatcher, SomfyRtsViewControl);
}

bool somfy_rts_scene_control_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void somfy_rts_scene_control_on_exit(void* context) {
    UNUSED(context);
}
