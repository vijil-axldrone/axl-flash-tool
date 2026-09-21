#ifndef PACKET_SERIAL_H
#define PACKET_SERIAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Includes
 * ============================================================================ */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ============================================================================
 * Types and Data Structures
 * ============================================================================ */
typedef enum {
    PACKET_OK = 0,
    PACKET_ERROR,
    PACKET_TIMEOUT,
    PACKET_WRONG_CRC,
    PACKET_WRONG_HEADER,
    PACKET_WRONG_END,
    PACKET_WRONG_LENGTH,
} packet_status_t;

typedef void (*packet_receive_cb)(const uint8_t *data, size_t data_len);
typedef void (*packet_transmit_cb)(const uint8_t *data, size_t data_len);
typedef int (*packet_available_cb)(void);
typedef uint8_t (*packet_read_cb)(void);
typedef uint32_t (*packet_millis_cb)(void);

typedef enum {
    RX_STATE_WAIT_SYNC1,
    RX_STATE_WAIT_SYNC2,
    RX_STATE_READ_HEADER,
    RX_STATE_READ_PAYLOAD,
    RX_STATE_READ_CRC1,
    RX_STATE_READ_CRC2,
    RX_STATE_WAIT_END
} packet_rx_state_t;

typedef struct {
    packet_transmit_cb tx_func;
    packet_available_cb available_func;
    packet_read_cb read_func;
    packet_millis_cb millis_func;

    uint32_t ack_timeout_ms;
    packet_receive_cb callback;

    uint8_t *rx_buffer;
    size_t rx_buffer_size;
    size_t rx_data_len;

    uint8_t current_msg_id;
    uint8_t expected_chunk_id;

    packet_rx_state_t rx_state;

    uint8_t rx_header[6];
    uint8_t rx_header_index;

    uint8_t rx_chunk_payload[64];
    uint16_t rx_payload_index;

    uint8_t rx_crc_bytes[2];
} reliable_packeter_t;

/* ============================================================================
 * Public Function Prototypes
 * ============================================================================ */
void packeter_init(reliable_packeter_t *ctx, uint32_t ack_timeout_ms);
void packeter_set_callbacks(reliable_packeter_t *ctx,
                            packet_transmit_cb tx_func,
                            packet_available_cb available_func,
                            packet_read_cb read_func,
                            packet_millis_cb millis_func);
void packeter_on_receive(reliable_packeter_t *ctx, packet_receive_cb callback);
void packeter_set_receive_buffer(reliable_packeter_t *ctx, uint8_t *buffer, size_t buffer_size);

packet_status_t packeter_send(reliable_packeter_t *ctx, const uint8_t *data, size_t data_len);
void packeter_update(reliable_packeter_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* PACKET_SERIAL_H */
