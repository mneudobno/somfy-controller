#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/view.h>
#include <gui/modules/text_input.h>

#define SOMFY_RTS_MAX_BLINDS    16
#define SOMFY_RTS_NAME_MAX      32
#define SOMFY_RTS_ALL_BLINDS    0xFF

typedef struct {
    char name[SOMFY_RTS_NAME_MAX];
    uint32_t address;
    uint16_t rolling_code;
} SomfyBlind;

typedef enum {
    SomfyRtsViewMenu,
    SomfyRtsViewControl,
    SomfyRtsViewTextInput,
    SomfyRtsViewConfirmDelete,
} SomfyRtsView;

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    Submenu* submenu;
    View* control_view;
    TextInput* text_input;
    DialogEx* dialog_ex;
    SomfyBlind blinds[SOMFY_RTS_MAX_BLINDS];
    uint8_t blind_count;
    uint8_t selected_blind;
    char text_buf[SOMFY_RTS_NAME_MAX];
} SomfyRtsApp;

typedef enum {
    SomfyRtsSceneMenu,
    SomfyRtsSceneControl,
    SomfyRtsSceneAdd,
    SomfyRtsSceneConfirmDelete,
    SomfyRtsSceneCount,
} SomfyRtsScene;

typedef enum {
    SomfyRtsCustomEventMenuSelected,
    SomfyRtsCustomEventAddBlind,
    SomfyRtsCustomEventTextInputDone,
    SomfyRtsCustomEventDeleteBlind,
    SomfyRtsCustomEventDeleteConfirm,
    SomfyRtsCustomEventDeleteCancel,
} SomfyRtsCustomEvent;
