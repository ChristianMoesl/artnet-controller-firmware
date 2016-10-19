/**
 * @file
 *
 * @copyright Copyright (c) 2015 Christian Moesl. All rights reserved.
 */
#include <assert.h>

#include "asf.h"
#include "memory.h"


#define CRC16_CCITT_POLYNOM 0x1021

static void soft_crc16ccitt (uint16_t *crc16Reg, uint8_t dataByte);


tEepromStatus readEepromStream(uint16_t addr, uint8_t *buffer, uint16_t dataLen, uint8_t dataVersion)
{
    assert(buffer != NULL);
   
    uint16_t checksum = 0;
    
    /* Read and check data length */
    uint16_t readDataLen = (uint16_t)nvm_eeprom_read_byte(addr++) << 8;
    readDataLen += nvm_eeprom_read_byte(addr++);
    if (readDataLen != dataLen || readDataLen <= EEPROM_HEADER_AND_TRAILER_LEN)
    {
        return EEPROM_WRONG_DATA_LEN;
    }        
    soft_crc16ccitt(&checksum, readDataLen >> 8);
    soft_crc16ccitt(&checksum, readDataLen);
    
   
    /* Read and check data version */
    uint8_t readVersion = nvm_eeprom_read_byte(addr++);
    if(readVersion != dataVersion)
    {
        return EEPROM_WRONG_DATA_VERSION;
    }        
    soft_crc16ccitt(&checksum, readVersion);
       
       
    /* Read data stream */
    for(uint16_t dataPos = 0; dataPos < (dataLen - EEPROM_HEADER_AND_TRAILER_LEN); dataPos++)
    {
        uint8_t readDataByte = nvm_eeprom_read_byte(addr++);
        soft_crc16ccitt(&checksum, readDataByte);
        buffer[dataPos] = readDataByte;
    }
    
    
    /* Read and check checksum */
    uint16_t readChecksum = (uint16_t)nvm_eeprom_read_byte(addr++) << 8;
    readChecksum += nvm_eeprom_read_byte(addr++);
    if(readChecksum != checksum)
    {
        return EEPROM_WRONG_CHECKSUM;
    }
    
    return EEPROM_ALL_OK;
}




tEepromStatus writeEepromStream(uint16_t addr, const uint8_t *data, uint16_t dataLen, uint8_t dataVersion)
{
    assert(data != NULL);
    
    uint16_t checksum = 0;
    
    soft_crc16ccitt(&checksum, dataLen >> 8);
    nvm_eeprom_write_byte(addr++, dataLen >> 8);
    soft_crc16ccitt(&checksum, dataLen);
    nvm_eeprom_write_byte(addr++, dataLen);
    soft_crc16ccitt(&checksum, dataVersion);
    nvm_eeprom_write_byte(addr++, dataVersion);
    
    for(uint16_t pos = 0; pos < dataLen; pos++)
    {
        soft_crc16ccitt(&checksum, data[pos]);
        nvm_eeprom_write_byte(addr++, data[pos]);
    }
    
    nvm_eeprom_write_byte(addr++, checksum >> 8);
    nvm_eeprom_write_byte(addr++, checksum);
    
    return EEPROM_ALL_OK;
}




/*  Function: soft_crc16ccitt
 * 	Function Author: Andreas Steinbacher.
 *  Additional Authors: Inspired by "http://srecord.sourceforge.net/crc16-ccitt.html"
 *  Description: Calculates the CRC-16 CCITT of the given buffer. It is independent of the type of interface the data is read from (FLASH, IO ...)
 *  Input: Pointer to crc shift register, pointer to crc polynomial, pointer to the current byte block of the data stream
 *  Output: CRC-16 CCITT in the given crc16Reg pointer.
 *  Side Effects: None observed.
 */
static void soft_crc16ccitt (uint16_t *crc16Reg, uint8_t data)
{
	uint8_t xorFlag = 0;				//Flag to store xor result
    uint16_t ByteN = (uint16_t)data << 8;

	for (uint8_t i = 0; i < 8; i++)								//For each and every bit in the byte block...
	{
		if ((*crc16Reg ^ ByteN) & 0x8000)		//Test if most left bit of the crc shift register is high
		{
			xorFlag= 1;								//If the bit is one, rise the flag.
		}
		else
		{
			xorFlag= 0;								//If the bit is zero, lower the flag.
		}
		*crc16Reg = *crc16Reg << 1;					//The crc register is shifted left for every bit in the data stream. The last bit is zero now.

		if (xorFlag)								//Check the xor flag that was set if the most left bit of the CRC was one at the last bit check.
		{
			*crc16Reg = *crc16Reg ^ CRC16_CCITT_POLYNOM;	//A polynomial division is easy in the binary system.
		}
		ByteN = ByteN << 1;							//Shift data block one bit left
	}
}
