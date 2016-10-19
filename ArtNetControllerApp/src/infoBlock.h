
#ifndef INFOBLOCK_H_
#define INFOBLOCK_H_

#include <stdint.h>

#include "asf.h"

// definitions
#define INFO_BLOCK_ID_0             'I'
#define INFO_BLOCK_ID_1             'B'
#define INFO_BLOCK_VERSION          0x10
#define MAX_LEN_OF_INFO_BLOCK       64   // bytes
#define MAX_LEN_OF_VERSION_STRING   33    // bytes

#define USER_SIGNATURE_POS_BOOTER_INFO_BLOCK    0
#define USER_SIGNATURE_POS_APP_INFO_BLOCK       64

#define INFO_BLOCK_POS_ID                     0
#define INFO_BLOCK_POS_VERSION                2
#define INFO_BLOCK_POS_INFO_BLOCK_LEN         3
#define INFO_BLOCK_POS_START_ADDR             4
#define INFO_BLOCK_POS_FW_LEN                 8
#define INFO_BLOCK_POS_FW_CRC                 12
#define INFO_BLOCK_POS_FW_ID                  14
#define INFO_BLOCK_POS_FW_VERSION             18
#define INFO_BLOCK_POS_FW_BUILD_YEAR          22
#define INFO_BLOCK_POS_FW_BUILD_MONTH         23
#define INFO_BLOCK_POS_FW_BUILD_DAY           24
#define INFO_BLOCK_POS_FW_BUILD_HOUR          25
#define INFO_BLOCK_POS_FW_BUILD_MINUTE        26
#define INFO_BLOCK_POS_FW_BUILD_SECOND        27
#define INFO_BLOCK_POS_FW_VERSION_STRING_LEN  28
#define INFO_BLOCK_POS_FW_VERSION_STRING      29

#define INFO_BLOCK_TYPE_BOOTER  'B'
#define INFO_BLOCK_TYPE_APPLI   'A'

typedef struct
{
    uint8_t type;
    uint8_t isValid;
    uint32_t fwLen;
    uint16_t fwCrc;
    uint8_t fwId[4];
    uint32_t fwVersion;
    uint16_t buildYear;
    uint8_t buildMonth;
    uint8_t buildDay;
    uint8_t buildHour;
    uint8_t buildMinute;
    uint8_t buildSecond;
    uint8_t fwVersionStringLen;
    uint8_t fwVersionString[MAX_LEN_OF_VERSION_STRING];
    uint16_t infoBlockCrc;
    uint8_t infoBockLen;
}tInfoBlockXMega;


static inline uint8_t readMajorFwVersion(void)
{
    return nvm_read_user_signature_row(USER_SIGNATURE_POS_APP_INFO_BLOCK + INFO_BLOCK_POS_FW_VERSION);
}
static inline uint8_t readMinorFwVersion(void)
{
    return nvm_read_user_signature_row(USER_SIGNATURE_POS_APP_INFO_BLOCK + INFO_BLOCK_POS_FW_VERSION + 1);
}

#endif /* 01_MAIN_H_ */
