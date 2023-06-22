#include "../nfc_app_i.h"

enum SubmenuIndex {
    SubmenuIndexReadCardType,
    SubmenuIndexMfClassicKeys,
    SubmenuIndexMfUltralightUnlock,
};

void nfc_scene_extra_actions_submenu_callback(void* context, uint32_t index) {
    NfcApp* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_extra_actions_on_enter(void* context) {
    NfcApp* nfc = context;
    Submenu* submenu = nfc->submenu;

    submenu_add_item(
        submenu,
        "Read Specific Card Type",
        SubmenuIndexReadCardType,
        nfc_scene_extra_actions_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Mifare Classic Keys",
        SubmenuIndexMfClassicKeys,
        nfc_scene_extra_actions_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Unlock NTAG/Ultralight",
        SubmenuIndexMfUltralightUnlock,
        nfc_scene_extra_actions_submenu_callback,
        nfc);
    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(nfc->scene_manager, NfcSceneExtraActions));
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_extra_actions_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexMfClassicKeys) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneNotImplemented);
            consumed = true;
        } else if(event.event == SubmenuIndexMfUltralightUnlock) {
            mf_ultralight_auth_reset(nfc->mf_ul_auth);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightUnlockMenu);
            consumed = true;
        } else if(event.event == SubmenuIndexReadCardType) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSelectProtocol);
            consumed = true;
        }
        scene_manager_set_scene_state(nfc->scene_manager, NfcSceneExtraActions, event.event);
    }

    return consumed;
}

void nfc_scene_extra_actions_on_exit(void* context) {
    NfcApp* nfc = context;

    submenu_reset(nfc->submenu);
}
