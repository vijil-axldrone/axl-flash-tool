/* ============================================================================
 * Includes
 * ============================================================================ */
#include "packet_serial.h"

#include <string.h>
#include "crc.h"

/* ============================================================================
 * Macros / Defines
 * ============================================================================ */
#define SYNC1             0xAA
#define SYNC2             0xBB
#define END_BYTE          0xAB

#define FLAG_LAST_CHUNK   0x01
#define FLAG_ACK          0x02
#define FLAG_NACK         0x04

#define MAX_PAYLOAD_SIZE  64
#define HEADER_SIZE       6

/* ============================================================================
 * External Declarations
 * ============================================================================ */
/* hcrc is a STM32 HAL handle — on Linux, the HAL_CRC_Calculate macro in
 * crc.h redirects to sw_crc16(), so this dummy variable is never used. */
static CRC_HandleTypeDef hcrc __attribute__((unused));



/* ============================================================================
 * Private Function Prototypes
 * ============================================================================ */
static void reset_rx_state(reliable_packeter_t *ctx);
static void send_ack(reliable_packeter_t *ctx, uint8_t msg_id, uint8_t chunk_id, bool is_ack);
static void process_received_chunk(reliable_packeter_t *ctx);

/* ============================================================================
 * Private Function Implementations
 * ============================================================================ */
static void reset_rx_state(reliable_packeter_t *ctx) {
    ctx->rx_state = RX_STATE_WAIT_SYNC1;
    ctx->rx_header_index = 0;
    ctx->rx_payload_index = 0;
}

static void send_ack(reliable_packeter_t *ctx, uint8_t msg_id, uint8_t chunk_id, bool is_ack) {
    if (!ctx || !ctx->tx_func) return;

    uint8_t ack_frame[HEADER_SIZE + 3];
    ack_frame[0] = SYNC1;
    ack_frame[1] = SYNC2;
    ack_frame[2] = msg_id;
    ack_frame[3] = chunk_id;
    ack_frame[4] = is_ack ? FLAG_ACK : FLAG_NACK;
    ack_frame[5] = 0;

    uint16_t crc = (uint16_t)HAL_CRC_Calculate(&hcrc, (uint32_t *)ack_frame, HEADER_SIZE);
    ack_frame[HEADER_SIZE] = (crc >> 8) & 0xFF;
    ack_frame[HEADER_SIZE + 1] = crc & 0xFF;
    ack_frame[HEADER_SIZE + 2] = END_BYTE;

    ctx->tx_func(ack_frame, sizeof(ack_frame));
}

static void process_received_chunk(reliable_packeter_t *ctx) {
    uint8_t msg_id = ctx->rx_header[2];
    uint8_t chunk_id = ctx->rx_header[3];
    uint8_t flags = ctx->rx_header[4];
    uint8_t len = ctx->rx_header[5];

    uint16_t expected_crc = (uint16_t)HAL_CRC_Calculate(&hcrc, (uint32_t *)ctx->rx_header, HEADER_SIZE);
    if (len > 0) {
        uint8_t temp_buf[HEADER_SIZE + MAX_PAYLOAD_SIZE];
        memcpy(temp_buf, ctx->rx_header, HEADER_SIZE);
        memcpy(temp_buf + HEADER_SIZE, ctx->rx_chunk_payload, len);
        expected_crc = (uint16_t)HAL_CRC_Calculate(&hcrc, (uint32_t *)temp_buf, HEADER_SIZE + len);
    }

    uint16_t received_crc = ((uint16_t)ctx->rx_crc_bytes[0] << 8) | ctx->rx_crc_bytes[1];

    if (expected_crc != received_crc) {
        send_ack(ctx, msg_id, chunk_id, false);
        return;
    }

    send_ack(ctx, msg_id, chunk_id, true);

    if (msg_id != ctx->current_msg_id || chunk_id == 0) {
        ctx->current_msg_id = msg_id;
        ctx->expected_chunk_id = 0;
        ctx->rx_data_len = 0;
    }

    if (chunk_id == ctx->expected_chunk_id) {
        if (ctx->rx_buffer && ctx->rx_buffer_size >= ctx->rx_data_len + len) {
            if (len > 0) {
                memcpy(ctx->rx_buffer + ctx->rx_data_len, ctx->rx_chunk_payload, len);
                ctx->rx_data_len += len;
            }
            ctx->expected_chunk_id++;

            if (flags & FLAG_LAST_CHUNK) {
                if (ctx->callback) {
                    ctx->callback(ctx->rx_buffer, ctx->rx_data_len);
                }
                ctx->expected_chunk_id = 0;
            }
        }
    }
}

/* ============================================================================
 * Public Function Implementations
 * ============================================================================ */
void packeter_init(reliable_packeter_t *ctx, uint32_t ack_timeout_ms) {
    if (!ctx) return;

    ctx->tx_func = NULL;
    ctx->available_func = NULL;
    ctx->read_func = NULL;
    ctx->millis_func = NULL;

    ctx->ack_timeout_ms = ack_timeout_ms;
    ctx->callback = NULL;

    ctx->rx_buffer = NULL;
    ctx->rx_buffer_size = 0;
    ctx->rx_data_len = 0;

    ctx->current_msg_id = 0;
    ctx->expected_chunk_id = 0;

    reset_rx_state(ctx);
}

void packeter_set_callbacks(reliable_packeter_t *ctx,
                            packet_transmit_cb tx_func,
                            packet_available_cb available_func,
                            packet_read_cb read_func,
                            packet_millis_cb millis_func) {
    if (!ctx) return;
    ctx->tx_func = tx_func;
    ctx->available_func = available_func;
    ctx->read_func = read_func;
    ctx->millis_func = millis_func;
}

void packeter_on_receive(reliable_packeter_t *ctx, packet_receive_cb callback) {
    if (!ctx) return;
    ctx->callback = callback;
}

void packeter_set_receive_buffer(reliable_packeter_t *ctx, uint8_t *buffer, size_t buffer_size) {
    if (!ctx) return;
    ctx->rx_buffer = buffer;
    ctx->rx_buffer_size = buffer_size;
}

packet_status_t packeter_send(reliable_packeter_t *ctx, const uint8_t *data, size_t data_len) {
    if (!ctx || !data || data_len == 0 || !ctx->tx_func || !ctx->millis_func || !ctx->available_func || !ctx->read_func) {
        return PACKET_ERROR;
    }

    ctx->current_msg_id++;

    uint16_t total_chunks = (data_len + MAX_PAYLOAD_SIZE - 1) / MAX_PAYLOAD_SIZE;
    uint8_t frame_buffer[MAX_PAYLOAD_SIZE + HEADER_SIZE + 3];

    for (uint16_t chunk_idx = 0; chunk_idx < total_chunks; chunk_idx++) {
        size_t offset = chunk_idx * MAX_PAYLOAD_SIZE;
        size_t chunk_payload_size = data_len - offset;
        if (chunk_payload_size > MAX_PAYLOAD_SIZE) {
            chunk_payload_size = MAX_PAYLOAD_SIZE;
        }

        bool is_last = (chunk_idx == total_chunks - 1);

        frame_buffer[0] = SYNC1;
        frame_buffer[1] = SYNC2;
        frame_buffer[2] = ctx->current_msg_id;
        frame_buffer[3] = (uint8_t)(chunk_idx & 0xFF);
        frame_buffer[4] = is_last ? FLAG_LAST_CHUNK : 0x00;
        frame_buffer[5] = (uint8_t)chunk_payload_size;

        for (size_t i = 0; i < chunk_payload_size; i++) {
            frame_buffer[HEADER_SIZE + i] = data[offset + i];
        }

        size_t crc_calc_len = HEADER_SIZE + chunk_payload_size;
        uint16_t crc = (uint16_t)HAL_CRC_Calculate(&hcrc, (uint32_t *)frame_buffer, crc_calc_len);

        frame_buffer[crc_calc_len] = (crc >> 8) & 0xFF;
        frame_buffer[crc_calc_len + 1] = crc & 0xFF;
        frame_buffer[crc_calc_len + 2] = END_BYTE;

        size_t total_frame_len = crc_calc_len + 3;

        uint8_t retry = 3;
        bool ack_received = false;

        while (retry-- > 0 && !ack_received) {
            ctx->tx_func(frame_buffer, total_frame_len);

            uint32_t start_time = ctx->millis_func();
            bool waiting = true;

            uint8_t ack_state = 0;
            uint8_t ack_msg = 0;
            uint8_t ack_chunk = 0;
            uint8_t ack_flags = 0;

            while ((ctx->millis_func() - start_time) < ctx->ack_timeout_ms && waiting) {
                while (ctx->available_func() && waiting) {
                    uint8_t b = ctx->read_func();
                    switch (ack_state) {
                    case 0: if (b == SYNC1) ack_state = 1; break;
                    case 1: if (b == SYNC2) ack_state = 2; else ack_state = 0; break;
                    case 2: ack_msg = b; ack_state = 3; break;
                    case 3: ack_chunk = b; ack_state = 4; break;
                    case 4: ack_flags = b; ack_state = 5; break;
                    case 5: ack_state = (b == 0) ? 6 : 0; break;
                    case 6: ack_state = 7; break;
                    case 7: ack_state = 8; break;
                    case 8:
                        if (b == END_BYTE) {
                            if (ack_msg == ctx->current_msg_id && ack_chunk == (uint8_t)(chunk_idx & 0xFF)) {
                                if (ack_flags & FLAG_ACK) {
                                    ack_received = true;
                                    waiting = false;
                                } else if (ack_flags & FLAG_NACK) {
                                    waiting = false;
                                }
                            }
                        }
                        ack_state = 0;
                        break;
                    }
                }
            }
        }

        if (!ack_received) {
            return PACKET_TIMEOUT;
        }
    }

    return PACKET_OK;
}

void packeter_update(reliable_packeter_t *ctx) {
    if (!ctx || !ctx->available_func || !ctx->read_func) return;

    while (ctx->available_func()) {
        uint8_t b = ctx->read_func();

        switch (ctx->rx_state) {
        case RX_STATE_WAIT_SYNC1:
            if (b == SYNC1) {
                ctx->rx_header[0] = b;
                ctx->rx_header_index = 1;
                ctx->rx_state = RX_STATE_WAIT_SYNC2;
            }
            break;

        case RX_STATE_WAIT_SYNC2:
            if (b == SYNC2) {
                ctx->rx_header[1] = b;
                ctx->rx_header_index = 2;
                ctx->rx_state = RX_STATE_READ_HEADER;
            } else if (b == SYNC1) {
                ctx->rx_header_index = 1;
            } else {
                ctx->rx_state = RX_STATE_WAIT_SYNC1;
            }
            break;

        case RX_STATE_READ_HEADER:
            ctx->rx_header[ctx->rx_header_index++] = b;
            if (ctx->rx_header_index == HEADER_SIZE) {
                if (ctx->rx_header[4] & (FLAG_ACK | FLAG_NACK)) {
                    ctx->rx_state = RX_STATE_WAIT_SYNC1;
                    break;
                }
                if (ctx->rx_header[5] > 0) {
                    ctx->rx_payload_index = 0;
                    ctx->rx_state = RX_STATE_READ_PAYLOAD;
                } else {
                    ctx->rx_state = RX_STATE_READ_CRC1;
                }
            }
            break;

        case RX_STATE_READ_PAYLOAD:
            ctx->rx_chunk_payload[ctx->rx_payload_index++] = b;
            if (ctx->rx_payload_index == ctx->rx_header[5]) {
                ctx->rx_state = RX_STATE_READ_CRC1;
            }
            break;

        case RX_STATE_READ_CRC1:
            ctx->rx_crc_bytes[0] = b;
            ctx->rx_state = RX_STATE_READ_CRC2;
            break;

        case RX_STATE_READ_CRC2:
            ctx->rx_crc_bytes[1] = b;
            ctx->rx_state = RX_STATE_WAIT_END;
            break;

        case RX_STATE_WAIT_END:
            if (b == END_BYTE) {
                process_received_chunk(ctx);
            }
            reset_rx_state(ctx);
            break;
        }
    }
}
