/**
 * @file nfc_poller.h
 * @brief The NfcPoller library provides NFC card reading(polling) facilities.
 * @see nfc_scanner.h
 *
 * Once started, it will try to activate and read a card using the designated protocol,
 * which is usually obtained by creating and starting an NfcScanner first.
 *
 * When running, NfcPoller will generate events that the calling code must handle
 * by providing a callback function. The events passed to the callback are protocol-specific
 * and may include errors, state changes, data reception, special function requests and more.
 *
 */
#pragma once

#include <nfc/protocols/nfc_generic_event.h>
#include <nfc/protocols/nfc_device_base.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NfcPoller opaque type definition.
 */
typedef struct NfcPoller NfcPoller;

/**
 * @brief Allocate an NfcPoller instance.
 *
 * @param[in] nfc pointer to an Nfc instance (@see nfc.h).
 * @param[in] protocol identifier of the protocol to be used.
 * @returns pointer to an allocated instance.
 */
NfcPoller* nfc_poller_alloc(Nfc* nfc, NfcProtocol protocol);

/**
 * @brief Delete an NfcPoller instance.
 *
 * @param[in,out] instance pointer to the instance to be deleted.
 */
void nfc_poller_free(NfcPoller* instance);

/**
 * @brief Start an NfcPoller instance.
 *
 * The callback logic is protocol-specific, so it cannot be described here in detail.
 * However, the callback return value ALWAYS determines what the poller should do next:
 * to continue whatever it was doing prior to the callback run or to stop.
 *
 * @param[in,out] instance pointer to the instance to be started.
 * @param[in] callback pointer to a user-defined callback function which will receive events.
 * @param[in] context pointer to a user-specific context (will be passed to the callback).j
 */
void nfc_poller_start(NfcPoller* instance, NfcGenericCallback callback, void* context);

/**
 * @brief Stop an NfcPoller instance.
 *
 * The reading process can be stopped explicitly (the other way is via the callback return value).
 *
 * @param[in,out] instance pointer to the instance to be stopped.
 */
void nfc_poller_stop(NfcPoller* instance);

/**
 * @brief Detect whether there is a card supporting a particular protocol in the vicinity.
 *
 * The behaviour of this function is protocol-defined, in general, it will do whatever is
 * necessary to determine whether a card supporting the current protocol is in the vicinity
 * and whether it is functioning normally.
 *
 * It is used automatically inside NfcScanner (@see nfc_scanner.h), so there is usually no need
 * to call it explicitly.
 *
 * @param[in,out] instance pointer to the instance to perform the detection with.
 * @returns true if a supported card was detected, false otherwise.
 */
bool nfc_poller_detect(NfcPoller* instance);

/**
 * @brief Get the protocol identifier an NfcPoller instance was created with.
 *
 * @param[in] instance pointer to the instance to be queried.
 * @returns identifier of the protocol used by the instance.
 */
NfcProtocol nfc_poller_get_protocol(const NfcPoller* instance);

/**
 * @brief Get the data that was that was gathered during the reading process.
 *
 * @param[in] instance pointer to the instance to be queried.
 * @returns pointer to the NFC device data.
 */
const NfcDeviceData* nfc_poller_get_data(const NfcPoller* instance);

#ifdef __cplusplus
}
#endif
