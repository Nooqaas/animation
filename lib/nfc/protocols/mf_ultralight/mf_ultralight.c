#include "mf_ultralight.h"

#include <furi.h>

#define MF_ULTRALIGHT_PROTOCOL_NAME "NTAG/Ultralight"

typedef struct {
    uint16_t total_pages;
    uint16_t config_page;
    uint32_t feature_set;
} MfUltralightFeatures;

static const uint32_t mf_ultralight_data_format_version = 1;

static const MfUltralightFeatures mf_ultralight_features[MfUltralightTypeNum] = {
    [MfUltralightTypeUnknown] =
        {
            .total_pages = 16,
            .config_page = 0,
            .feature_set = MfUltralightFeatureSupportCompatibleWrite,
        },
    [MfUltralightTypeNTAG203] =
        {
            .total_pages = 42,
            .config_page = 0,
            .feature_set = MfUltralightFeatureSupportCompatibleWrite |
                           MfUltralightFeatureSupportCounterInMemory,
        },
    [MfUltralightTypeUL11] =
        {
            .total_pages = 20,
            .config_page = 16,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportReadSignature |
                MfUltralightFeatureSupportReadCounter |
                MfUltralightFeatureSupportCheckTearingFlag | MfUltralightFeatureSupportFastRead |
                MfUltralightFeatureSupportIncCounter | MfUltralightFeatureSupportCompatibleWrite |
                MfUltralightFeatureSupportAuthentication | MfUltralightFeatureSupportVcsl,
        },
    [MfUltralightTypeUL21] =
        {
            .total_pages = 41,
            .config_page = 37,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportReadSignature |
                MfUltralightFeatureSupportReadCounter |
                MfUltralightFeatureSupportCheckTearingFlag | MfUltralightFeatureSupportFastRead |
                MfUltralightFeatureSupportIncCounter | MfUltralightFeatureSupportCompatibleWrite |
                MfUltralightFeatureSupportAuthentication | MfUltralightFeatureSupportVcsl,
        },
    [MfUltralightTypeNTAG213] =
        {
            .total_pages = 45,
            .config_page = 41,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportReadSignature |
                MfUltralightFeatureSupportReadCounter | MfUltralightFeatureSupportFastRead |
                MfUltralightFeatureSupportCompatibleWrite |
                MfUltralightFeatureSupportAuthentication |
                MfUltralightFeatureSupportSingleCounter | MfUltralightFeatureSupportAsciiMirror,
        },
    [MfUltralightTypeNTAG215] =
        {
            .total_pages = 135,
            .config_page = 131,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportReadSignature |
                MfUltralightFeatureSupportReadCounter | MfUltralightFeatureSupportFastRead |
                MfUltralightFeatureSupportCompatibleWrite |
                MfUltralightFeatureSupportAuthentication |
                MfUltralightFeatureSupportSingleCounter | MfUltralightFeatureSupportAsciiMirror,
        },
    [MfUltralightTypeNTAG216] =
        {
            .total_pages = 231,
            .config_page = 227,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportReadSignature |
                MfUltralightFeatureSupportReadCounter | MfUltralightFeatureSupportFastRead |
                MfUltralightFeatureSupportCompatibleWrite |
                MfUltralightFeatureSupportAuthentication |
                MfUltralightFeatureSupportSingleCounter | MfUltralightFeatureSupportAsciiMirror,
        },
    [MfUltralightTypeNTAGI2C1K] =
        {
            .total_pages = 231,
            .config_page = 0,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportFastRead |
                MfUltralightFeatureSupportFastWrite | MfUltralightFeatureSupportSectorSelect,
        },
    [MfUltralightTypeNTAGI2C2K] =
        {
            .total_pages = 485,
            .config_page = 0,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportFastRead |
                MfUltralightFeatureSupportFastWrite | MfUltralightFeatureSupportSectorSelect,
        },
    [MfUltralightTypeNTAGI2CPlus1K] =
        {
            .total_pages = 236,
            .config_page = 227,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportReadSignature |
                MfUltralightFeatureSupportFastRead | MfUltralightFeatureSupportAuthentication |
                MfUltralightFeatureSupportSectorSelect,
        },
    [MfUltralightTypeNTAGI2CPlus2K] =
        {
            .total_pages = 492,
            .config_page = 227,
            .feature_set =
                MfUltralightFeatureSupportReadVersion | MfUltralightFeatureSupportReadSignature |
                MfUltralightFeatureSupportFastRead | MfUltralightFeatureSupportAuthentication |
                MfUltralightFeatureSupportSectorSelect,
        },
};

const NfcProtocolBase nfc_protocol_mf_ultralight = {
    .protocol_name = MF_ULTRALIGHT_PROTOCOL_NAME,
    .alloc = (NfcProtocolAlloc)mf_ultralight_alloc,
    .free = (NfcProtocolFree)mf_ultralight_free,
    .reset = (NfcProtocolReset)mf_ultralight_reset,
    .copy = (NfcProtocolCopy)mf_ultralight_copy,
    .verify = (NfcProtocolVerify)mf_ultralight_verify,
    .load = (NfcProtocolLoad)mf_ultralight_load,
    .save = (NfcProtocolSave)mf_ultralight_save,
    .is_equal = (NfcProtocolEqual)mf_ultralight_is_equal,
    .get_device_name = (NfcProtocolGetDeviceName)mf_ultralight_get_device_name,
    .get_uid = (NfcProtocolGetUid)mf_ultralight_get_uid,
};

MfUltralightData* mf_ultralight_alloc() {
    MfUltralightData* data = malloc(sizeof(MfUltralightData));
    data->nfca_data = nfca_alloc();
    return data;
}

void mf_ultralight_free(MfUltralightData* data) {
    furi_assert(data);

    nfca_free(data->nfca_data);
    free(data);
}

void mf_ultralight_reset(MfUltralightData* data) {
    furi_assert(data);

    nfca_reset(data->nfca_data);
}

void mf_ultralight_copy(MfUltralightData* data, const MfUltralightData* other) {
    furi_assert(data);
    furi_assert(other);

    nfca_copy(data->nfca_data, other->nfca_data);
    memcpy(data->counter, other->counter, MF_ULTRALIGHT_COUNTER_NUM);
    memcpy(data->tearing_flag, other->tearing_flag, MF_ULTRALIGHT_TEARING_FLAG_NUM);
    memcpy(data->page, other->page, MF_ULTRALIGHT_MAX_PAGE_NUM);

    data->type = other->type;
    data->version = other->version;
    data->signature = other->signature;

    data->pages_read = other->pages_read;
    data->pages_total = other->pages_total;
    data->auth_attempts = other->auth_attempts;
}

// TODO: Improve this function
static const char* mf_ultralight_get_device_name_by_type(MfUltralightType type, bool full_name) {
    // FIXME: Use a LUT instead of if/switch
    if(type == MfUltralightTypeNTAG213) {
        return "NTAG213";
    } else if(type == MfUltralightTypeNTAG215) {
        return "NTAG215";
    } else if(type == MfUltralightTypeNTAG216) {
        return "NTAG216";
    } else if(type == MfUltralightTypeNTAGI2C1K) {
        return "NTAG I2C 1K";
    } else if(type == MfUltralightTypeNTAGI2C2K) {
        return "NTAG I2C 2K";
    } else if(type == MfUltralightTypeNTAGI2CPlus1K) {
        return "NTAG I2C Plus 1K";
    } else if(type == MfUltralightTypeNTAGI2CPlus2K) {
        return "NTAG I2C Plus 2K";
    } else if(type == MfUltralightTypeNTAG203) {
        return "NTAG203";
    } else if(type == MfUltralightTypeUL11 && full_name) {
        return "Mifare Ultralight 11";
    } else if(type == MfUltralightTypeUL21 && full_name) {
        return "Mifare Ultralight 21";
    } else {
        return "Mifare Ultralight";
    }
}

bool mf_ultralight_verify(MfUltralightData* data, const FuriString* device_type) {
    furi_assert(data);

    bool verified = false;

    for(MfUltralightType i = 0; i < MfUltralightTypeNum; i++) {
        const char* name = mf_ultralight_get_device_name_by_type(i, true);
        verified = furi_string_equal_str(device_type, name);
        if(verified) {
            data->type = i;
            break;
        }
    }

    return verified;
}

bool mf_ultralight_load(MfUltralightData* data, FlipperFormat* ff, uint32_t version) {
    furi_assert(data);

    FuriString* temp_str = furi_string_alloc();
    bool parsed = false;

    do {
        // Read NFCA data
        if(!nfca_load_data(data->nfca_data, ff, version)) break;

        // Read Ultralight specific data
        // Read Mifare Ultralight format version
        uint32_t data_format_version = 0;
        if(!flipper_format_read_uint32(ff, "Data format version", &data_format_version, 1)) {
            if(!flipper_format_rewind(ff)) break;
        }

        // Read signature
        if(!flipper_format_read_hex(
               ff, "Signature", data->signature.data, sizeof(MfUltralightSignature)))
            break;
        // Read Mifare version
        if(!flipper_format_read_hex(
               ff, "Mifare version", (uint8_t*)&data->version, sizeof(MfUltralightVersion)))
            break;
        // Read counters and tearing flags
        bool counters_parsed = true;
        for(size_t i = 0; i < 3; i++) {
            furi_string_printf(temp_str, "Counter %d", i);
            if(!flipper_format_read_uint32(
                   ff, furi_string_get_cstr(temp_str), &data->counter[i].counter, 1)) {
                counters_parsed = false;
                break;
            }
            furi_string_printf(temp_str, "Tearing %d", i);
            if(!flipper_format_read_hex(
                   ff, furi_string_get_cstr(temp_str), data->tearing_flag[i].data, 1)) {
                counters_parsed = false;
                break;
            }
        }
        if(!counters_parsed) break;
        // Read pages
        uint32_t pages_total = 0;
        if(!flipper_format_read_uint32(ff, "Pages total", &pages_total, 1)) break;
        uint32_t pages_read = 0;
        if(data_format_version < mf_ultralight_data_format_version) {
            pages_read = pages_total;
        } else {
            if(!flipper_format_read_uint32(ff, "Pages read", &pages_read, 1)) break;
        }
        data->pages_total = pages_total;
        data->pages_read = pages_read;

        if((pages_read > MF_ULTRALIGHT_MAX_PAGE_NUM) || (pages_total > MF_ULTRALIGHT_MAX_PAGE_NUM))
            break;

        bool pages_parsed = true;
        for(size_t i = 0; i < pages_total; i++) {
            furi_string_printf(temp_str, "Page %d", i);
            if(!flipper_format_read_hex(
                   ff,
                   furi_string_get_cstr(temp_str),
                   data->page[i].data,
                   sizeof(MfUltralightPage))) {
                pages_parsed = false;
                break;
            }
        }
        if(!pages_parsed) break;

        // Read authentication counter
        if(!flipper_format_read_uint32(
               ff, "Failed authentication attempts", &data->auth_attempts, 1)) {
            data->auth_attempts = 0;
        }

        parsed = true;
    } while(false);

    furi_string_free(temp_str);

    return parsed;
}

bool mf_ultralight_save(const MfUltralightData* data, FlipperFormat* ff, uint32_t version) {
    furi_assert(data);

    FuriString* temp_str = furi_string_alloc();
    bool saved = false;

    do {
        const char* device_type_name = mf_ultralight_get_device_name_by_type(data->type, true);
        if(!flipper_format_write_string_cstr(ff, "Device type", device_type_name)) break;
        if(!nfca_save_data(data->nfca_data, ff, version)) break;
        if(!flipper_format_write_comment_cstr(ff, "Mifare Ultralight specific data")) break;
        if(!flipper_format_write_uint32(
               ff, "Data format version", &mf_ultralight_data_format_version, 1))
            break;
        if(!flipper_format_write_hex(
               ff, "Signature", data->signature.data, sizeof(MfUltralightSignature)))
            break;
        if(!flipper_format_write_hex(
               ff, "Mifare version", (uint8_t*)&data->version, sizeof(MfUltralightVersion)))
            break;

        // Write conters and tearing flags data
        bool counters_saved = true;
        for(size_t i = 0; i < 3; i++) {
            furi_string_printf(temp_str, "Counter %d", i);
            if(!flipper_format_write_uint32(
                   ff, furi_string_get_cstr(temp_str), &data->counter[i].counter, 1)) {
                counters_saved = false;
                break;
            }
            furi_string_printf(temp_str, "Tearing %d", i);
            if(!flipper_format_write_hex(
                   ff, furi_string_get_cstr(temp_str), data->tearing_flag->data, 1)) {
                counters_saved = false;
                break;
            }
        }
        if(!counters_saved) break;

        // Write pages data
        uint32_t pages_total = data->pages_total;
        uint32_t pages_read = data->pages_read;
        if(!flipper_format_write_uint32(ff, "Pages total", &pages_total, 1)) break;
        if(!flipper_format_write_uint32(ff, "Pages read", &pages_read, 1)) break;
        bool pages_saved = true;
        for(size_t i = 0; i < data->pages_total; i++) {
            furi_string_printf(temp_str, "Page %d", i);
            if(!flipper_format_write_hex(
                   ff,
                   furi_string_get_cstr(temp_str),
                   data->page[i].data,
                   sizeof(MfUltralightPage))) {
                pages_saved = false;
                break;
            }
        }
        if(!pages_saved) break;

        // Write authentication counter
        if(!flipper_format_write_uint32(
               ff, "Failed authentication attempts", &data->auth_attempts, 1))
            break;

        saved = true;
    } while(false);

    furi_string_free(temp_str);

    return saved;
}

bool mf_ultralight_is_equal(const MfUltralightData* data, const MfUltralightData* other) {
    // TODO: Complete equality method
    return nfca_is_equal(data->nfca_data, other->nfca_data);
}

// TODO: Improve this function
const char*
    mf_ultralight_get_device_name(const MfUltralightData* data, NfcProtocolNameType name_type) {
    furi_assert(data);
    furi_assert(data->type < MfUltralightTypeNum);

    return mf_ultralight_get_device_name_by_type(data->type, name_type == NfcProtocolNameTypeFull);
}

const uint8_t* mf_ultralight_get_uid(const MfUltralightData* data, size_t* uid_len) {
    furi_assert(data);

    return nfca_get_uid(data->nfca_data, uid_len);
}

MfUltralightType mf_ultralight_get_type_by_version(MfUltralightVersion* version) {
    furi_assert(version);

    MfUltralightType type = MfUltralightTypeUnknown;

    if(version->storage_size == 0x0B || version->storage_size == 0x00) {
        type = MfUltralightTypeUL11;
    } else if(version->storage_size == 0x0E) {
        type = MfUltralightTypeUL21;
    } else if(version->storage_size == 0x0F) {
        type = MfUltralightTypeNTAG213;
    } else if(version->storage_size == 0x11) {
        type = MfUltralightTypeNTAG215;
    } else if(version->prod_subtype == 5 && version->prod_ver_major == 2) {
        if(version->prod_ver_minor == 1) {
            if(version->storage_size == 0x13) {
                type = MfUltralightTypeNTAGI2C1K;
            } else if(version->storage_size == 0x15) {
                type = MfUltralightTypeNTAGI2C2K;
            }
        } else if(version->prod_ver_minor == 2) {
            if(version->storage_size == 0x13) {
                type = MfUltralightTypeNTAGI2CPlus1K;
            } else if(version->storage_size == 0x15) {
                type = MfUltralightTypeNTAGI2CPlus2K;
            }
        }
    } else if(version->storage_size == 0x13) {
        type = MfUltralightTypeNTAG216;
    }

    return type;
}

uint16_t mf_ultralight_get_pages_total(MfUltralightType type) {
    return mf_ultralight_features[type].total_pages;
}

uint32_t mf_ultralight_get_feature_support_set(MfUltralightType type) {
    return mf_ultralight_features[type].feature_set;
}

bool mf_ultralight_detect_protocol(const NfcaData* nfca_data) {
    furi_assert(nfca_data);

    bool mfu_detected = (nfca_data->atqa[0] == 0x44) && (nfca_data->atqa[1] == 0x00) &&
                        (nfca_data->sak == 0x00);

    return mfu_detected;
}

uint16_t mf_ultralight_get_config_page_num(MfUltralightType type) {
    return mf_ultralight_features[type].config_page;
}

bool mf_ultralight_get_config_page(const MfUltralightData* data, MfUltralightConfigPages** config) {
    furi_assert(data);
    furi_assert(config);

    bool config_pages_found = false;

    uint16_t config_page = mf_ultralight_features[data->type].config_page;
    if(config_page != 0) {
        *config = (MfUltralightConfigPages*)&data->page[config_page];
        config_pages_found = true;
    }

    return config_pages_found;
}

bool mf_ultralight_is_all_data_read(const MfUltralightData* data) {
    furi_assert(data);

    bool all_read = false;
    if(data->pages_read == data->pages_total) {
        // Having read all the pages doesn't mean that we've got everything.
        // By default PWD is 0xFFFFFFFF, but if read back it is always 0x00000000,
        // so a default read on an auth-supported NTAG is never complete.
        uint32_t feature_set = mf_ultralight_get_feature_support_set(data->type);
        if(feature_set & MfUltralightFeatureSupportAuthentication) {
            all_read = true;
        } else {
            MfUltralightConfigPages* config = NULL;
            if(mf_ultralight_get_config_page(data, &config)) {
                all_read = ((config->password.pass != 0) || (config->pack.pack != 0));
            }
        }
    }

    return all_read;
}

bool mf_ultralight_is_counter_configured(const MfUltralightData* data) {
    furi_assert(data);

    MfUltralightConfigPages* config = NULL;
    bool configured = false;

    switch(data->type) {
    case MfUltralightTypeNTAG213:
    case MfUltralightTypeNTAG215:
    case MfUltralightTypeNTAG216:
        if(mf_ultralight_get_config_page(data, &config)) {
            configured = config->access.nfc_cnt_en;
        }
        break;

    default:
        configured = true;
        break;
    }

    return configured;
}
