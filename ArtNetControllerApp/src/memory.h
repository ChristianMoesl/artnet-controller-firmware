/**
 * @file
 *
 * @copyright Copyright (c) 2015 Christian Moesl. All rights reserved.
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define EEPROM_HEADER_LEN       3   //length in big indian + data version + id
#define EEPROM_HEADER_POS_LEN       0
#define EEPROM_HEADER_POS_VERSION   2

#define EEPROM_TRAILER_LEN      2   //16bit crc
#define EEPROM_HEADER_POS_CRC   0

#define EEPROM_HEADER_AND_TRAILER_LEN   (EEPROM_HEADER_LEN + EEPROM_TRAILER_LEN)
#define EEPROM_LEN_OF_ID        2


//////////////////////////////////////////////////////////////////////////////////////////////
///////////////         === Memory Map ===      
//////////////////////////////////////////////////////////////////////////////////////////////
#define EEPROM_TOUCH_DIM_ID         "TD"
#define EEPROM_TOUCH_DIM_ADDR       0
#define EEPROM_TOUCH_DIM_VERSION    0x10
#define EEPROM_TOUCH_DIM_LEN        3

#define EEPROM_NET_PARAMS_ID        "NP"
#define EEPROM_NET_PARAMS_ADDR      (EEPROM_TOUCH_DIM_ADDR + EEPROM_TOUCH_DIM_LEN + EEPROM_HEADER_AND_TRAILER_LEN)
#define EEPROM_NET_PARAMS_VERSION   0x10
#define EEPROM_NET_PARAMS_LEN       12

#define EEPROM_ETH_ADDRESS_ID           "ET"
#define EEPROM_ETH_ADDRESS_ADDR         (EEPROM_NET_PARAMS_ADDR + EEPROM_NET_PARAMS_LEN + EEPROM_HEADER_AND_TRAILER_LEN)
#define EEPROM_ETH_ADDRESS_VERSION      0x10
#define EEPROM_ETH_ADDRESS_LEN          6

#define EEPROM_ART_FW_UPDATE_IP_ID      "FU"
#define EEPROM_ART_FW_UPDATE_IP_ADDR    (EEPROM_ETH_ADDRESS_ADDR + EEPROM_ETH_ADDRESS_LEN + EEPROM_HEADER_AND_TRAILER_LEN)
#define EEPROM_ART_FW_UPDATE_IP_VERSION 0x10
#define EEPROM_ART_FW_UPDATE_IP_LEN     4

#define EEPROM_ART_ADDRESS_STRUCT_ID        "AA"
#define EEPROM_ART_ADDRESS_STRUCT_ADDR      (EEPROM_ART_FW_UPDATE_IP_ADDR + EEPROM_ART_FW_UPDATE_IP_LEN + EEPROM_HEADER_AND_TRAILER_LEN)
#define EEPROM_ART_ADDRESS_STRUCT_VERSION   0x10
#define EEPROM_ART_ADDRESS_STRUCT_LEN       10

#define EEPROM_ART_NODE_STATUS_STRUCT_ID        "AS"
#define EEPROM_ART_NODE_STATUS_STRUCT_ADDR      (EEPROM_ART_ADDRESS_STRUCT_ADDR + EEPROM_ART_ADDRESS_STRUCT_LEN + EEPROM_HEADER_AND_TRAILER_LEN)
#define EEPROM_ART_NODE_STATUS_STRUCT_VERSION   0x10
#define EEPROM_ART_NODE_STATUS_STRUCT_LEN       9
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


typedef enum
{
    EEPROM_ALL_OK,
    EEPROM_WRONG_CHECKSUM,
    EEPROM_WRONG_DATA_LEN,
    EEPROM_WRONG_DATA_VERSION,    
}tEepromStatus;


tEepromStatus readEepromStream(uint16_t addr, uint8_t *buffer, uint16_t dataLen, uint8_t dataVersion);
tEepromStatus writeEepromStream(uint16_t addr, const uint8_t *data, uint16_t dataLen, uint8_t dataVersion);


#ifdef __cplusplus
}
#endif

#endif /* MEMORY_H_ */
