#pragma once

#include "mf_ultralight_poller.h"
#include <lib/nfc/protocols/nfca/nfca_poller_i.h>
#include <nfc/helpers/nfc_poller_buffer.h>
#include <lib/nfc/protocols/nfc_util.h>

#define MF_ULTRALIGHT_POLLER_STANDART_FWT_FC (60000)

typedef struct {
    MfUltralightPage page;
    uint8_t page_to_write;
} MfUltralightPollerWritePageCommand;

typedef union {
    uint8_t page_to_read;
    uint8_t counter_to_read;
    uint8_t tearing_flag_to_read;
    MfUltralightPollerWritePageCommand write_cmd;
} MfUltralightPollerContextData;

typedef enum {
    MfUltralightPollerStateIdle,
    MfUltralightPollerStateReadVersion,
    MfUltralightPollerStateDetectNtag203,
    MfUltralightPollerStateGetFeatureSet,
    MfUltralightPollerStateReadSignature,
    MfUltralightPollerStateReadCounters,
    MfUltralightPollerStateReadTearingFlags,
    MfUltralightPollerStateAuth,
    MfUltralightPollerStateReadPages,
    MfUltralightPollerStateReadFailed,
    MfUltralightPollerStateReadSuccess,

    MfUltralightPollerStateNum,
} MfUltralightPollerState;

typedef enum {
    MfUltralightPollerSessionStateIdle,
    MfUltralightPollerSessionStateActive,
    MfUltralightPollerSessionStateStopRequest,
} MfUltralightPollerSessionState;

struct MfUltralightPoller {
    NfcaPoller* nfca_poller;
    MfUltralightPollerSessionState session_state;
    MfUltralightPollerState state;
    NfcPollerBuffer* buffer;
    MfUltralightData* data;
    MfUltralightPollerCallback callback;
    MfUltralightAuthPassword auth_password;
    uint32_t feature_set;
    uint16_t pages_read;
    uint16_t pages_total;
    void* context;
};

MfUltralightError mf_ultralight_process_error(NfcaError error);

MfUltralightError
    mf_ultralight_poller_async_auth(MfUltralightPoller* instance, MfUltralightAuthPassword* data);

MfUltralightError mf_ultralight_poller_async_read_page(MfUltralightPoller* instance, uint8_t page);

MfUltralightError mf_ultralight_poller_async_write_page(
    MfUltralightPoller* instance,
    uint8_t page,
    MfUltralightPage* data);

MfUltralightError mf_ultralight_poller_async_read_version(MfUltralightPoller* instance);

MfUltralightError mf_ultralight_poller_async_read_signature(MfUltralightPoller* instance);

MfUltralightError
    mf_ultralight_poller_async_read_counter(MfUltralightPoller* instance, uint8_t counter_num);

MfUltralightError mf_ultralight_poller_async_read_tearing_flag(
    MfUltralightPoller* instance,
    uint8_t tearing_falg_num);

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
