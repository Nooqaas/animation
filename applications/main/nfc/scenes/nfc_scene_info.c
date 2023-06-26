#include "../nfc_app_i.h"

#include "../helpers/format/nfc_protocol_format.h"

static void
    nfc_scene_nfc_info_widget_callback(GuiButtonType result, InputType type, void* context) {
    NfcApp* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_info_on_enter(void* context) {
    NfcApp* nfc = context;
    Widget* widget = nfc->widget;

    uint8_t text_scroll_height = 0;
    if(nfc_protocol_format_get_features(nfc->nfc_dev) & NfcProtocolFormatFeatureMoreData) {
        widget_add_button_element(
            widget, GuiButtonTypeRight, "More", nfc_scene_nfc_info_widget_callback, nfc);
        text_scroll_height = 52;
    } else {
        text_scroll_height = 64;
    }

    FuriString* temp_str;
    temp_str = furi_string_alloc();

    nfc_protocol_format_info(nfc->nfc_dev, NfcProtocolFormatTypeFull, temp_str);

    widget_add_text_scroll_element(
        widget, 0, 0, 128, text_scroll_height, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

static const NfcScene nfc_scene_info_data_scenes[NfcProtocolTypeMax] = {
    [NfcProtocolTypeIso14443_3a] = NfcSceneNotImplemented,
    [NfcProtocolTypeIso14443_4a] = NfcSceneNotImplemented,
    [NfcProtocolTypeMfUltralight] = NfcSceneNotImplemented,
    [NfcProtocolTypeMfClassic] = NfcSceneNotImplemented,
    [NfcProtocolTypeMfDesfire] = NfcSceneMfDesfireData,
};

bool nfc_scene_info_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            const NfcProtocolType protocol_type = nfc_dev_get_protocol_type(nfc->nfc_dev);
            const NfcScene menu_scene = nfc_scene_info_data_scenes[protocol_type];
            scene_manager_next_scene(nfc->scene_manager, menu_scene);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_info_on_exit(void* context) {
    NfcApp* nfc = context;
    widget_reset(nfc->widget);
}
