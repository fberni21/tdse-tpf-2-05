/*
 * @file   : eeprom.c
 * @date   : Jan 19, 2026
 * @author : Franco Berni <fberni@fi.uba.ar>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/
#include "main.h"
#include "eeprom.h"

#include <stdbool.h>

/********************** macros and definitions *******************************/

#define EEEPROM_I2C_ADDRESS	0xA0
#define EEPROM_TIMEOUT_MS	1000
#define EEPROM_INTERNAL_WRITE_MS 5

/********************** internal data declaration ****************************/

static volatile bool eeprom_writing = false;
static uint32_t write_start_tick = 0;

/********************** internal functions declaration ***********************/

static bool eeprom_is_busy(void);

/********************** internal data definition *****************************/

/********************** external data declaration ****************************/

extern I2C_HandleTypeDef hi2c1;

/********************** external functions definition ************************/

// La escritura se tiene que hacer de forma asíncrona porque ocurre durante los updates.
HAL_StatusTypeDef eeprom_write_async(uint8_t offset, void *data, size_t size)
{
    if (eeprom_is_busy())
    {
        return HAL_BUSY;
    }

    HAL_StatusTypeDef status = HAL_I2C_Mem_Write_IT(&hi2c1, EEEPROM_I2C_ADDRESS,
                                                   offset, I2C_MEMADD_SIZE_16BIT,
                                                   (uint8_t*)data, size);

    if (HAL_OK == status)
    {
        eeprom_writing = true;
    }

    return status;
}

// La lectura puede ser bloqueante porque solo ocurre al principio
void eeprom_read(uint8_t offset, void *data, size_t size)
{
	HAL_I2C_Mem_Read(&hi2c1, EEEPROM_I2C_ADDRESS, offset, I2C_MEMADD_SIZE_16BIT, data, size, EEPROM_TIMEOUT_MS);
}

static bool eeprom_is_busy(void) {
    if (eeprom_writing)
    {
        if ((HAL_GetTick() - write_start_tick) >= EEPROM_INTERNAL_WRITE_MS)
        {
            eeprom_writing = false;
        }
    }
    return eeprom_writing;
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1) {
        write_start_tick = HAL_GetTick();
    }
}

/********************** end of file ******************************************/
