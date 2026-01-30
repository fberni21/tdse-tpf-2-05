/*
 * @file   : eeprom.c
 * @date   : Jan 19, 2026
 * @author : Franco Berni <fberni@fi.uba.ar>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/
#include "main.h"
#include "utils.h"
#include "task_adc.h"

/********************** macros and definitions *******************************/

#define TEMP_SENSOR_MAX		100		// celsius
#define PRESS_SENSOR_MAX	110		// kPa

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data declaration ****************************/

/********************** external functions definition ************************/

bool is_in_range(uint32_t value, uint32_t min, uint32_t max)
{
	return (value >= min) && (value <= max);
}

uint32_t temp_raw_to_celsius(uint32_t temp_raw)
{
	return (temp_raw * TEMP_SENSOR_MAX) / ADC_MAX_VALUE;
}

uint32_t press_raw_to_kPa(uint32_t press_raw)
{
	return (press_raw * PRESS_SENSOR_MAX) / ADC_MAX_VALUE;
}

/********************** end of file ******************************************/
