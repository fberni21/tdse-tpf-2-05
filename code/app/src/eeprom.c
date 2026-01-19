/*
 * @file   : eeprom.c
 * @date   : Jan 19, 2026
 * @author : Franco Berni <fberni@fi.uba.ar>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/
#include "main.h"
#include "eeprom.h"

/********************** macros and definitions *******************************/

#define EEEPROM_I2C_ADDRESS	0xA0
#define EEPROM_TIMEOUT_MS	1000

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data declaration ****************************/

extern I2C_HandleTypeDef hi2c1;

/********************** external functions definition ************************/

void eeprom_write(uint8_t offset, void *data, size_t size)
{
	HAL_I2C_Mem_Write(&hi2c1, EEEPROM_I2C_ADDRESS, offset, I2C_MEMADD_SIZE_16BIT, data, size, EEPROM_TIMEOUT_MS);
}

void eeprom_read(uint8_t offset, void *data, size_t size)
{
	HAL_I2C_Mem_Read(&hi2c1, EEEPROM_I2C_ADDRESS, offset, I2C_MEMADD_SIZE_16BIT, data, size, EEPROM_TIMEOUT_MS);
}

/********************** end of file ******************************************/
