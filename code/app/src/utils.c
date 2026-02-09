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

#include <string.h>

/********************** macros and definitions *******************************/

#define TEMP_SENSOR_MAX		100		// celsius
#define PRESS_SENSOR_MAX	110		// kPa

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

const char units_lut[128] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1',
		'2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5',
		'6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '1',
		'2', '3', '4', '5', '6', '7', };

const char tens_lut[128] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
		'1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '2', '2', '2', '2',
		'2', '2', '2', '2', '2', '2', '3', '3', '3', '3', '3', '3', '3', '3',
		'3', '3', '4', '4', '4', '4', '4', '4', '4', '4', '4', '4', '5', '5',
		'5', '5', '5', '5', '5', '5', '5', '5', '6', '6', '6', '6', '6', '6',
		'6', '6', '6', '6', '7', '7', '7', '7', '7', '7', '7', '7', '7', '7',
		'8', '8', '8', '8', '8', '8', '8', '8', '8', '8', '9', '9', '9', '9',
		'9', '9', '9', '9', '9', '9', '0', '0', '0', '0', '0', '0', '0', '0',
		'0', '0', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '2', '2',
		'2', '2', '2', '2', '2', '2', };

const char status_msg[17] = "   \xDF""C |     kPa ";

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

void build_status_bar(char out_str[17], uint32_t temp, uint32_t press) {
	memcpy(out_str, status_msg, sizeof(status_msg));
	out_str[1] = tens_lut[temp];
	out_str[2] = units_lut[temp];
	if (press >= 100)
		out_str[8] = '1';

	out_str[9] = tens_lut[press];
	out_str[10] = units_lut[press];
}

/********************** end of file ******************************************/
