#include "can_hacker2.h"
#include "can_hacker2_cmd.h"
#include "can_hacker2_utils.h"

#include "../mcp251xfd_interconnect/can0_function.h"
#include "../mcp251xfd_interconnect/show_device.h"
#include "../mcp251xfd/MCP251XFD.h"

//https://www.ostrovni-elektrarny.cz/docs/Lawicell_can232_v3.pdf

#define TAG "CanHacker2"
#define CAN_HACKER2_TX_RX_BUF_SIZE 1024
#define CAN_HACKER2_CMD_BUF_SIZE 128
#define CAN_HACKER2_PAYLOAD_DATA_SIZE 8

struct CanHacker2 {
    FuriStreamBuffer* stream_rx;
    FuriStreamBuffer* stream_tx;
    bool exit;

    uint8_t cmd_buf[CAN_HACKER2_CMD_BUF_SIZE];
    size_t cmd_buf_len;
    bool can_open;
    bool can_timestamp;
    bool can_listen_only;
    uint32_t seq_counter;
    CanHacker2Bitrate can0_bitrate;

    MCP251XFD_CANMessage rx_msg;
    uint8_t rx_msg_payload_data[CAN_HACKER2_PAYLOAD_DATA_SIZE];
    uint32_t rx_msg_time_stamp;

    CanHacker2Callback callback;
    void* context;
};

CanHacker2* can_hacker2_alloc(void) {
    CanHacker2* instance = malloc(sizeof(CanHacker2));
    instance->stream_rx =
        furi_stream_buffer_alloc(sizeof(int8_t) * CAN_HACKER2_TX_RX_BUF_SIZE, sizeof(int8_t));
    instance->stream_tx =
        furi_stream_buffer_alloc(sizeof(int8_t) * CAN_HACKER2_TX_RX_BUF_SIZE, sizeof(int8_t));
    instance->exit = false;
    memset(instance->cmd_buf, 0, sizeof(uint8_t) * CAN_HACKER2_CMD_BUF_SIZE);
    instance->cmd_buf_len = 0;
    instance->can_open = false;
    instance->can0_bitrate = CanHacker2BitrateOff;
    instance->can_timestamp = false;
    instance->can_listen_only = false;
    instance->rx_msg.PayloadData = &instance->rx_msg_payload_data[0];
    //instance->rx_msg.MessageSEQ = &instance->seq_counter;
    instance->seq_counter = 0;
    return instance;
}

void can_hacker2_free(CanHacker2* instance) {
    furi_assert(instance);
    if(instance->can_open) {
        can0_function_device_deinit();
        instance->can_open = false;
    }
    furi_stream_buffer_free(instance->stream_tx);
    furi_stream_buffer_free(instance->stream_rx);

    free(instance);
}

void can_hacker2_set_callback(CanHacker2* instance, CanHacker2Callback callback, void* context) {
    furi_assert(instance);

    instance->callback = callback;
    instance->context = context;
}

size_t can_hacker2_spaces_rx(CanHacker2* instance) {
    return furi_stream_buffer_spaces_available(instance->stream_rx);
}

bool can_hacker2_rx(CanHacker2* instance, uint8_t* data, size_t len) {
    furi_assert(instance);
    furi_assert(data);
    furi_assert(len != 0);
    size_t ret = furi_stream_buffer_send(instance->stream_rx, data, sizeof(uint8_t) * len, 0);
    return ret == sizeof(uint8_t) * len;
}

size_t can_hacker2_tx(CanHacker2* instance, uint8_t* data, size_t max_len) {
    furi_assert(instance);
    return furi_stream_buffer_receive(instance->stream_tx, data, sizeof(int8_t) * max_len, 0);
}

static void can_hacker2_tx_add_data(CanHacker2* instance, uint8_t* data, size_t len) {
    furi_assert(instance);
    furi_stream_buffer_send(instance->stream_tx, data, sizeof(uint8_t) * len, FuriWaitForever);
}

static void can_hacker2_tx_add_ch(CanHacker2* instance, uint8_t data) {
    furi_assert(instance);
    furi_stream_buffer_send(instance->stream_tx, &data, sizeof(uint8_t), FuriWaitForever);
}

void can_hacker2_exit(CanHacker2* instance) {
    furi_assert(instance);
    instance->exit = true;
}

// static uint8_t can_hacker2_getch(CanHacker2* instance) {
//     furi_assert(instance);
//     uint8_t data[1] = {0};
//     while(furi_stream_buffer_receive(instance->stream_rx, &data, sizeof(int8_t), 30) == 0) {
//         if(instance->exit) break;
//     };
//     return data[0];
// }

void can_hacker2_get_cmd(CanHacker2* instance) {
    furi_assert(instance);
    uint8_t data[1] = {0};

    // while(furi_stream_buffer_receive(instance->stream_rx, &data, sizeof(int8_t), 30) == 0) {
    //     if(instance->exit) break;
    // };

    //Todo: fix if translate work can interrupt
    if(furi_stream_buffer_receive(instance->stream_rx, &data, sizeof(int8_t), 0) == 0) {
        return;
    }
    switch(data[0]) {
    case CH2_CR:
    case CH2_LF:
        instance->cmd_buf[instance->cmd_buf_len] = CH2_END_OF_CMD;
        can_hacker2_process_cmd(instance);
        instance->cmd_buf_len = 0;
        break;
    case CH2_END_OF_CMD:
        break;

    default:
        if(instance->cmd_buf_len < CAN_HACKER2_CMD_BUF_SIZE) {
            instance->cmd_buf[instance->cmd_buf_len] = data[0];
            instance->cmd_buf_len++;
        }
        break;
    }
}

static void can_hacker2_can_close(CanHacker2* instance) {
    furi_assert(instance);
    if(instance->can_open) {
        can0_function_device_deinit();
        instance->can_open = false;
        can_hacker2_tx_add_ch(instance, CH2_OK);
    } else {
        can_hacker2_tx_add_ch(instance, CH2_BELL);
    }
}

static void can_hacker2_set_bitrate(CanHacker2* instance) {
    furi_assert(instance);
    furi_assert(instance->cmd_buf[0] == CH2_SET_BITRATE);
    if(instance->can_open) {
        can_hacker2_tx_add_ch(instance, CH2_BELL);
    } else {
        switch(instance->cmd_buf[1]) {
        case '0':
            FURI_LOG_D(TAG, "Set Bitrate: 10K");
            instance->can0_bitrate = CanHacker2Bitrate10K;
            break;
        case '1':
            FURI_LOG_D(TAG, "Set Bitrate: 20K");
            instance->can0_bitrate = CanHacker2Bitrate20K;
            break;
        case '2':
            FURI_LOG_D(TAG, "Set Bitrate: 50K");
            instance->can0_bitrate = CanHacker2Bitrate50K;
            break;
        case '3':
            FURI_LOG_D(TAG, "Set Bitrate: 100K");
            instance->can0_bitrate = CanHacker2Bitrate100K;
            break;
        case '4':
            FURI_LOG_D(TAG, "Set Bitrate: 125K");
            instance->can0_bitrate = CanHacker2Bitrate125K;
            break;
        case '5':
            FURI_LOG_D(TAG, "Set Bitrate: 250K");
            instance->can0_bitrate = CanHacker2Bitrate250K;
            break;
        case '6':
            FURI_LOG_D(TAG, "Set Bitrate: 500K");
            instance->can0_bitrate = CanHacker2Bitrate500K;
            break;
        case '7':
            FURI_LOG_D(TAG, "Set Bitrate: 800K");
            instance->can0_bitrate = CanHacker2Bitrate800K;
            break;
        case '8':
            FURI_LOG_D(TAG, "Set Bitrate: 1M");
            instance->can0_bitrate = CanHacker2Bitrate1M;
            break;
        default:
            FURI_LOG_D(TAG, "Set Bitrate: Off");
            instance->can0_bitrate = CanHacker2BitrateOff;
            break;
        }
        can_hacker2_tx_add_ch(instance, CH2_OK);
    }
}
static void can_hacker2_set_timestamp(CanHacker2* instance) {
    furi_assert(instance);
    furi_assert(instance->cmd_buf[0] == CH2_TIME_STAMP);

    switch(instance->cmd_buf[1]) {
    case '0':
        FURI_LOG_D(TAG, "Set Timestamp: Off");
        instance->can_timestamp = false;
        can_hacker2_tx_add_ch(instance, CH2_OK);
        break;
    case '1':
        FURI_LOG_D(TAG, "Set Timestamp: On");
        instance->can_timestamp = true;
        can_hacker2_tx_add_ch(instance, CH2_OK);
        break;
    default:
        FURI_LOG_D(TAG, "Set Timestamp: Unknown");
        can_hacker2_tx_add_ch(instance, CH2_BELL);
        break;
    }
}

static void can_hacher2_can_open(CanHacker2* instance) {
    furi_assert(instance);
    furi_assert(instance->cmd_buf[0] == CH2_OPEN_CAN_CHAN);

    if(instance->can_open) {
        can_hacker2_tx_add_ch(instance, CH2_BELL);

    } else {
        if(instance->can0_bitrate == CanHacker2BitrateOff) {
            //Todo: check supported speeds
            can_hacker2_tx_add_ch(instance, CH2_BELL);
        } else {
            //Todo: add loopback mode
            eERRORRESULT ret = can0_function_device_init_can20(instance->can0_bitrate);
            if(ret == ERR_OK) {
                show_device_detected(CAN0, can0_sysclk);
                show_device_configuration(&can0_bt_stats);
                show_device_fifo_configuration(&can0_fifo_list[0], CAN0_FIFO_COUNT);
                show_device_filter_configuration(&can_filter_list[0], CAN0_FILTER_COUNT, false);
                instance->can_open = true;
                can_hacker2_tx_add_ch(instance, CH2_OK);
            } else {
                can_hacker2_tx_add_ch(instance, CH2_BELL);
            }
        }
    }
}

static void can_hacker2_can_transmit(CanHacker2* instance) {
    furi_assert(instance);
    furi_assert(
        (instance->cmd_buf[0] == CH2_SEND_T11BIT_ID) ||
        (instance->cmd_buf[0] == CH2_SEND_T29BIT_ID) ||
        (instance->cmd_buf[0] == CH2_SEND_R11BIT_ID) ||
        (instance->cmd_buf[0] == CH2_SEND_R29BIT_ID));

    if(instance->can_open) {
        uint8_t is_exended = 0;
        uint8_t is_rtr = 0;

        switch(instance->cmd_buf[0]) {
        case CH2_SEND_T11BIT_ID:
            break;
        case CH2_SEND_T29BIT_ID:
            is_exended = 1;
            break;
        case CH2_SEND_R11BIT_ID:
            is_rtr = 1;
            break;
        case CH2_SEND_R29BIT_ID:
            is_exended = 1;
            is_rtr = 1;
            break;
        default:
            can_hacker2_tx_add_ch(instance, CH2_BELL);
            return;
        }

        uint8_t offset = 1;
        MCP251XFD_CANMessage tx_msg;
        uint8_t payload_data[8] = {0};
        tx_msg.PayloadData = &payload_data[0];
        tx_msg.MessageID = 0;
        tx_msg.ControlFlags = 0;
        uint8_t id_chars = is_exended ? 8 : 3;
        for(int i = 0; i < id_chars; i++) {
            tx_msg.MessageID <<= 4;
            tx_msg.MessageID += hexCharToByte(instance->cmd_buf[offset++]);
        }
        if(is_rtr) {
            tx_msg.ControlFlags |= MCP251XFD_REMOTE_TRANSMISSION_REQUEST;
        }
        // else {
        //     tx_msg.ControlFlags |= MCP251XFD_TRANSMIT_MESSAGE;
        // }
        if(is_exended) {
            tx_msg.ControlFlags |= MCP251XFD_CAN20_FRAME | MCP251XFD_EXTENDED_MESSAGE_ID;
        } else {
            tx_msg.ControlFlags |= MCP251XFD_CAN20_FRAME | MCP251XFD_STANDARD_MESSAGE_ID;
        }

        tx_msg.DLC = hexCharToByte(instance->cmd_buf[offset++]);
        if(tx_msg.DLC > 8 || (tx_msg.DLC == 0 && !is_rtr)) {
            FURI_LOG_E(TAG, "DLC > 8 or DLC == 0");
            can_hacker2_tx_add_ch(instance, CH2_BELL);
            return;
        }

        if(!is_rtr) {
            for(uint8_t i = 0; i < tx_msg.DLC; i++) {
                char hiHex = instance->cmd_buf[offset++];
                char loHex = instance->cmd_buf[offset++];
                payload_data[i] = hexCharToByte(loHex) + (hexCharToByte(hiHex) << 4);
            }
        }
        if(can0_function_transmit_msg(&tx_msg, &instance->seq_counter, MCP251XFD_TXQ, true) ==
           ERR_OK) {
            can_hacker2_tx_add_ch(instance, CH2_OK);
        } else {
            can_hacker2_tx_add_ch(instance, CH2_BELL);
        }
    } else {
        can_hacker2_tx_add_ch(instance, CH2_BELL);
    }
}

void can_hacker2_process_cmd(CanHacker2* instance) {
    furi_assert(instance);
    switch(instance->cmd_buf[0]) {
    case CH2_GET_SERIAL:
        can_hacker2_tx_add_data(
            instance, (uint8_t*)CH2_SERIAL_RESPONSE, strlen(CH2_SERIAL_RESPONSE));
        break;
    case CH2_GET_SW_VERSION:
        can_hacker2_tx_add_data(
            instance, (uint8_t*)CH2_SW_VERSION_RESPONSE, strlen(CH2_SW_VERSION_RESPONSE));
        break;
    case CH2_GET_VERSION:
        can_hacker2_tx_add_data(
            instance, (uint8_t*)CH2_VERSION_RESPONSE, strlen(CH2_VERSION_RESPONSE));
        break;

    case CH2_SEND_T11BIT_ID:
    case CH2_SEND_T29BIT_ID:
    case CH2_SEND_R11BIT_ID:
    case CH2_SEND_R29BIT_ID:
        can_hacker2_can_transmit(instance);
        break;
    case CH2_CLOSE_CAN_CHAN:
        can_hacker2_can_close(instance);
        break;
    case CH2_OPEN_CAN_CHAN:
        can_hacher2_can_open(instance);
        break;
    case CH2_SET_BITRATE:
        can_hacker2_set_bitrate(instance);
        break;
    case CH2_SET_ACR:
        //return receiveSetAcrCommand(buffer, length);
        break;
    case CH2_SET_AMR:
        //return receiveSetAmrCommand(buffer, length);
        break;
    case CH2_SET_BTR:
        //{
        //     if(isConnected()) {
        //         writeStream(BEL);
        //         writeDebugStream(F("SET_BTR command cannot be called while connected\n"));
        //         return ERROR_CONNECTED;
        //     }
        //     writeDebugStream(F("SET_BTR not supported\n"));
        //     return writeStream(CR);
        // }
        break;

    case CH2_LISTEN_ONLY:
        //return receiveListenOnlyCommand(buffer, length);
        break;
    case CH2_TIME_STAMP:
        can_hacker2_set_timestamp(instance);
        break;
    case CH2_WRITE_REG:
    case CH2_READ_REG:
        // {
        //     return writeStream(CR);
        // }
        break;
    case CH2_READ_STATUS:
    case CH2_READ_ECR:
    case CH2_READ_ALCR:
        // {
        //     if(!isConnected()) {
        //         writeDebugStream(F("Read status, ecr, alcr while not connected\n"));
        //         writeStream(BEL);
        //         return ERROR_NOT_CONNECTED;
        //     }
        //     return writeStream(CR);
        // }
        break;

    default:
        can_hacker2_tx_add_ch(instance, CH2_BELL);
        break;
    }
    if(instance->callback) {
        instance->callback(instance->context);
    }
}

// static void can_hacker2_can_to_canhacker2(CanHacker2* instance) {
//     furi_assert(instance);
//     furi_assert(msg);
//     furi_assert(msg->PayloadData);
//     furi_assert(msg->PayloadLength <= CAN_HACKER2_PAYLOAD_DATA_SIZE);

//     uint8_t data[32] = {0};

//     if(instance->recive_msg.ControlFlags & MCP251XFD_STANDARD_MESSAGE_ID){
//         sprintf(data, "r%03X%0", instance->recive_msg.MessageID, );
//     }

//     can_hacker2_tx_add_data(instance, data, len);
// }

void can_hacker2_process(CanHacker2* instance) {
    furi_assert(instance);
    can_hacker2_get_cmd(instance);

    if(instance->can_open) {
        can0_function_device_check_irq(); //check irq
        // bool recieve_event = false;
        // do{
        //     recieve_event = false;
        //     can0_function_receive_msg(CAN0, &receive_event, &instance->recive_msg, MCP251XFD_PAYLOAD_8BYTE, &instance->recive_msg_time_stamp);
        //     if(recieve_event){

        //     }
        // }
    }
}