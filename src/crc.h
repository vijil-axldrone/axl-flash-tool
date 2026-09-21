#ifndef CRC_H
#define CRC_H

#include <stdint.h>
#include <stddef.h>

/* Software CRC-16 (CRC-16/IBM) to replace STM32 HAL_CRC_Calculate.
 * The HAL function was called with byte arrays cast to uint32_t* — this
 * implementation accepts the same raw byte buffer + length. */
uint16_t sw_crc16(const uint8_t *data, size_t len);

/* Drop-in macro replacement so packet_serial.c needs no changes */
#define HAL_CRC_Calculate(hcrc_ptr, buf, len) sw_crc16((const uint8_t *)(buf), (len))

/* Dummy type so the extern declaration in packet_serial.c compiles */
typedef int CRC_HandleTypeDef;

#endif /* CRC_H */
