#include "somfy_rts.h"
#include "somfy_rts_store.h"
#include "scenes/somfy_rts_scene.h"

const AppSceneOnEnterCallback somfy_rts_scene_on_enter_handlers[] = {
    [SomfyRtsSceneMenu] = somfy_rts_scene_menu_on_enter,
    [SomfyRtsSceneControl] = somfy_rts_scene_control_on_enter,
    [SomfyRtsSceneAdd] = somfy_rts_scene_add_on_enter,
    [SomfyRtsSceneConfirmDelete] = somfy_rts_scene_confirm_delete_on_enter,
};

const AppSceneOnEventCallback somfy_rts_scene_on_event_handlers[] = {
    [SomfyRtsSceneMenu] = somfy_rts_scene_menu_on_event,
    [SomfyRtsSceneControl] = somfy_rts_scene_control_on_event,
    [SomfyRtsSceneAdd] = somfy_rts_scene_add_on_event,
    [SomfyRtsSceneConfirmDelete] = somfy_rts_scene_confirm_delete_on_event,
};

const AppSceneOnExitCallback somfy_rts_scene_on_exit_handlers[] = {
    [SomfyRtsSceneMenu] = somfy_rts_scene_menu_on_exit,
    [SomfyRtsSceneControl] = somfy_rts_scene_control_on_exit,
    [SomfyRtsSceneAdd] = somfy_rts_scene_add_on_exit,
    [SomfyRtsSceneConfirmDelete] = somfy_rts_scene_confirm_delete_on_exit,
};

const SceneManagerHandlers somfy_rts_scene_handlers = {
    .on_enter_handlers = somfy_rts_scene_on_enter_handlers,
    .on_event_handlers = somfy_rts_scene_on_event_handlers,
    .on_exit_handlers = somfy_rts_scene_on_exit_handlers,
    .scene_num = SomfyRtsSceneCount,
};

static bool somfy_rts_custom_event_callback(void* context, uint32_t event) {
    SomfyRtsApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool somfy_rts_back_event_callback(void* context) {
    SomfyRtsApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static SomfyRtsApp* somfy_rts_app_alloc(void) {
    SomfyRtsApp* app = malloc(sizeof(SomfyRtsApp));
    memset(app, 0, sizeof(SomfyRtsApp));

    app->gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, somfy_rts_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, somfy_rts_back_event_callback);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->scene_manager = scene_manager_alloc(&somfy_rts_scene_handlers, app);

    // Submenu (menu scene)
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(app->view_dispatcher, SomfyRtsViewMenu, submenu_get_view(app->submenu));

    // Control view (custom raw view with app pointer as model)
    app->control_view = view_alloc();
    view_allocate_model(app->control_view, ViewModelTypeLockFree, sizeof(SomfyRtsApp*));
    SomfyRtsApp** model = view_get_model(app->control_view);
    *model = app;
    view_commit_model(app->control_view, false);
    view_dispatcher_add_view(app->view_dispatcher, SomfyRtsViewControl, app->control_view);

    // Text input (add blind scene)
    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SomfyRtsViewTextInput, text_input_get_view(app->text_input));

    // Dialog (confirm delete)
    app->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SomfyRtsViewConfirmDelete, dialog_ex_get_view(app->dialog_ex));

    // Load saved blinds
    somfy_rts_store_load(app);

    return app;
}

static void somfy_rts_app_free(SomfyRtsApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, SomfyRtsViewMenu);
    view_dispatcher_remove_view(app->view_dispatcher, SomfyRtsViewControl);
    view_dispatcher_remove_view(app->view_dispatcher, SomfyRtsViewTextInput);
    view_dispatcher_remove_view(app->view_dispatcher, SomfyRtsViewConfirmDelete);

    submenu_free(app->submenu);
    view_free(app->control_view);
    text_input_free(app->text_input);
    dialog_ex_free(app->dialog_ex);

    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t somfy_rts_app(void* p) {
    UNUSED(p);

    SomfyRtsApp* app = somfy_rts_app_alloc();

    scene_manager_next_scene(app->scene_manager, SomfyRtsSceneMenu);
    view_dispatcher_run(app->view_dispatcher);

    somfy_rts_app_free(app);
    return 0;
}
