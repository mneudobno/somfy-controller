#pragma once

#include "../somfy_rts.h"
#include <gui/scene_manager.h>

// Scene on_enter handlers
void somfy_rts_scene_menu_on_enter(void* context);
void somfy_rts_scene_control_on_enter(void* context);
void somfy_rts_scene_add_on_enter(void* context);

// Scene on_event handlers
bool somfy_rts_scene_menu_on_event(void* context, SceneManagerEvent event);
bool somfy_rts_scene_control_on_event(void* context, SceneManagerEvent event);
bool somfy_rts_scene_add_on_event(void* context, SceneManagerEvent event);

// Scene on_exit handlers
void somfy_rts_scene_menu_on_exit(void* context);
void somfy_rts_scene_control_on_exit(void* context);
void somfy_rts_scene_add_on_exit(void* context);

extern const AppSceneOnEnterCallback somfy_rts_scene_on_enter_handlers[];
extern const AppSceneOnEventCallback somfy_rts_scene_on_event_handlers[];
extern const AppSceneOnExitCallback somfy_rts_scene_on_exit_handlers[];
extern const SceneManagerHandlers somfy_rts_scene_handlers;
