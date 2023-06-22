#include "../nfc_app_i.h"
#include <dolphin/dolphin.h>

void nfc_scene_detect_scan_callback(NfcScannerEvent event, void* context) {
    furi_assert(context);

    NfcApp* instance = context;

    if(event.type == NfcScannerEventTypeDetected) {
        instance->protocols_detected_num = event.data.protocol_num;
        memcpy(instance->protocols_detected, event.data.protocols, event.data.protocol_num);
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWorkerExit);
    }
}

void nfc_scene_detect_on_enter(void* context) {
    NfcApp* instance = context;

    // Setup view
    popup_reset(instance->popup);
    popup_set_text(
        instance->popup, "Apply card to\nFlipper's back", 97, 24, AlignCenter, AlignTop);
    popup_set_icon(instance->popup, 0, 8, &I_NFC_manual_60x50);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);

    nfc_scanner_start(instance->scanner, nfc_scene_detect_scan_callback, instance);

    nfc_blink_detect_start(instance);
}

bool nfc_scene_detect_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventWorkerExit) {
            if(instance->protocols_detected_num > 1) {
                scene_manager_next_scene(instance->scene_manager, NfcSceneSelectProtocol);
            } else {
                // TODO rework with generic read scene
                const uint32_t nfc_read_scenes[NfcProtocolTypeMax] = {
                    NfcSceneNfcaRead,
                    NfcSceneNfcaRead,
                    NfcSceneMfUltralightRead,
                    NfcSceneMfClassicDictAttack,
                    NfcSceneMfDesfireRead,
                };
                instance->protocols_detected_idx = 0;
                scene_manager_next_scene(
                    instance->scene_manager,
                    nfc_read_scenes[instance->protocols_detected[instance->protocols_detected_idx]]);
            }
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_detect_on_exit(void* context) {
    NfcApp* instance = context;

    nfc_scanner_stop(instance->scanner);
    popup_reset(instance->popup);

    nfc_blink_stop(instance);
}
