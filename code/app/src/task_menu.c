/*
 * Copyright (c) 2023 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * @file   : task_menu.c
 * @date   : Set 26, 2023
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/
/* Project includes. */
#include "main.h"

/* Demo includes. */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes. */
#include "board.h"
#include "app.h"
#include "task_menu_attribute.h"
#include "task_menu_interface.h"
#include "task_system_interface.h"
#include "task_display_interface.h"
#include "eeprom.h"
#include "utils.h"

/********************** macros and definitions *******************************/
#define G_TASK_MEN_CNT_INI			0ul
#define G_TASK_MEN_TICK_CNT_INI		0ul

#define DEL_MEN_XX_MIN				0ul
#define DEL_MEN_XX_MED				100ul
#define DEL_MEN_XX_MAX				500ul


#define TEMP_SETPOINT_INI		25 		// celsius
#define TEMP_HYSTERESIS_INI		2 		// celsius
#define PRESS_SETPOINT_INI		101 	// kPa
#define PRESS_HYSTERESIS_INI	1 		// kPa
#define ALARM_ENABLE_INI 		false

#define TEMP_SETPOINT_MIN		0		// celsius
#define TEMP_HYSTERESIS_MIN		1		// celsius
#define PRESS_SETPOINT_MIN		0		// kPa
#define PRESS_HYSTERESIS_MIN	1		// kPa

#define TEMP_SETPOINT_MAX		80		// celsius
#define TEMP_HYSTERESIS_MAX		10		// celsius
#define PRESS_SETPOINT_MAX		110		// kPa
#define PRESS_HYSTERESIS_MAX	10		// kPa


/********************** internal data declaration ****************************/
task_menu_dta_t task_menu_dta =
	{DEL_MEN_XX_MIN, ST_MEN_IDLE_VIEW, EV_MEN_ENT_IDLE, false, {0}, 0};

#define MENU_DTA_QTY	(sizeof(task_menu_dta)/sizeof(task_menu_dta_t))

#define MENU_CFG_ADDR 0

/********************** internal functions declaration ***********************/

void recover_saved_cfg();
void task_menu_statechart(shared_data_type *p_shared_data);

/********************** internal data definition *****************************/
const char *p_task_menu 		= "Task Menu (Interactive Menu)";
const char *p_task_menu_ 		= "Non-Blocking & Update By Time Code";

char menu_str[17] = {0};

/********************** external data declaration ****************************/
uint32_t g_task_menu_cnt;
volatile uint32_t g_task_menu_tick_cnt;

/********************** external functions definition ************************/
void task_menu_init(void *parameters)
{
	task_menu_dta_t *p_task_menu_dta;
	task_menu_st_t	state;
	task_menu_ev_t	event;
	bool b_event;

	shared_data_type *p_shared_data = (shared_data_type*)parameters;

	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_menu_init), p_task_menu);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_menu), p_task_menu_);

	g_task_menu_cnt = G_TASK_MEN_CNT_INI;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_menu_cnt), g_task_menu_cnt);

	init_queue_event_task_menu();

	/* Update Task Actuator Configuration & Data Pointer */
	p_task_menu_dta = &task_menu_dta;

	/* Print out: Task execution FSM */
	state = p_task_menu_dta->state;
	LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

	event = p_task_menu_dta->event;
	LOGGER_LOG("   %s = %lu", GET_NAME(event), (uint32_t)event);

	b_event = p_task_menu_dta->flag;
	LOGGER_LOG("   %s = %s\r\n", GET_NAME(b_event), (b_event ? "true" : "false"));

	recover_saved_cfg(&p_task_menu_dta->cfg);
	p_shared_data->cfg = p_task_menu_dta->cfg;

	g_task_menu_tick_cnt = G_TASK_MEN_TICK_CNT_INI;
}

void task_menu_update(void *parameters)
{
	bool b_time_update_required = false;

	/* Update Task Menu Counter */
	g_task_menu_cnt++;

	/* Protect shared resource (g_task_menu_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt)
    {
    	g_task_menu_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
		/* Protect shared resource (g_task_menu_tick) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_MEN_TICK_CNT_INI < g_task_menu_tick_cnt)
		{
			g_task_menu_tick_cnt--;
			b_time_update_required = true;
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

		task_menu_statechart((shared_data_type*)parameters);
	}
}

void task_menu_statechart(shared_data_type *p_shared_data)
{
	task_menu_dta_t *p_task_menu_dta;

	/* Update Task Menu Data Pointer */
	p_task_menu_dta = &task_menu_dta;

	if (DEL_MEN_XX_MIN < p_task_menu_dta->tick)
	{
		p_task_menu_dta->tick--;
	}
	else
	{
		p_task_menu_dta->tick = DEL_MEN_XX_MAX;

		if (true == any_event_task_menu())
		{
			p_task_menu_dta->flag = true;
			p_task_menu_dta->event = get_event_task_menu();
		}

		switch (p_task_menu_dta->state)
		{
		// ----------------------------------------------------------------
		// ESTADO 1: VISTA EN VIVO (IDLE)
		// ----------------------------------------------------------------
		case ST_MEN_IDLE_VIEW:
			//I2C_LCD_SetCursor(I2C_LCD_1, 0, 0);
			put_cmd_task_display(CMD_DISP_TO_LINE_0, NULL);
			snprintf(menu_str, sizeof(menu_str), "%2lu \xDF""C | %3lu kPa",
					 temp_raw_to_celsius(p_shared_data->temp_raw),
					 press_raw_to_kPa(p_shared_data->pressure_raw));
			put_cmd_task_display(CMD_DISP_WRITE_STR, menu_str);

			put_cmd_task_display(CMD_DISP_TO_LINE_1, NULL);
			put_cmd_task_display(CMD_DISP_WRITE_STR, "Enter p/ config");

			// Si presiona ENTER, va al menú principal
			if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			{
				p_task_menu_dta->flag = false;
				p_task_menu_dta->state = ST_MEN_MAIN_SELECT;
				p_task_menu_dta->current_selection = 0;
			}
			// Si no, volvemos al modo normal y para solo mostrar los valores actuales
			else if (true == p_task_menu_dta->flag)
			{
				put_event_task_system(EV_SYS_EXIT_MENU);
			}
			break;

		// ----------------------------------------------------------------
		// ESTADO 2: SELECCIÓN PRINCIPAL (Temp / Presion / Alarma)
		// ----------------------------------------------------------------
		case ST_MEN_MAIN_SELECT:
			put_cmd_task_display(CMD_DISP_TO_LINE_0, NULL);
			put_cmd_task_display(CMD_DISP_WRITE_STR, "Configurar:     ");

			put_cmd_task_display(CMD_DISP_TO_LINE_1, NULL);
			if (p_task_menu_dta->current_selection == 0)      put_cmd_task_display(CMD_DISP_WRITE_STR, "> Temperatura   ");
			else if (p_task_menu_dta->current_selection == 1) put_cmd_task_display(CMD_DISP_WRITE_STR, "> Presion       ");
			else if (p_task_menu_dta->current_selection == 2) put_cmd_task_display(CMD_DISP_WRITE_STR, "> Alarmas       ");

			if (true == p_task_menu_dta->flag)
			{
				p_task_menu_dta->flag = false;
				if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
				{
					// Cíclico: 0 -> 1 -> 2 -> 0
					p_task_menu_dta->current_selection = (p_task_menu_dta->current_selection + 1) % 3;
				}
				else if (EV_MEN_PRE_ACTIVE == p_task_menu_dta->event)
				{
					if (p_task_menu_dta->current_selection > 0)
						p_task_menu_dta->current_selection--;
					else
						p_task_menu_dta->current_selection = 2;
				}
				else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)
				{
					if (p_task_menu_dta->current_selection == 0) p_task_menu_dta->state = ST_MEN_TEMP_SELECT;
					else if (p_task_menu_dta->current_selection == 1) p_task_menu_dta->state = ST_MEN_PRESS_SELECT;
					else p_task_menu_dta->state = ST_MEN_ALARM_SELECT;

					p_task_menu_dta->current_selection = 0;
				}
				else if (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
				{
					// Volvemos al menú de vista en vivo y guardamos los cambios hechos en la EEPROM
					p_task_menu_dta->state = ST_MEN_IDLE_VIEW;
					eeprom_write(MENU_CFG_ADDR, &p_shared_data->cfg, sizeof(p_shared_data->cfg));
				}
			}
			break;

		// ----------------------------------------------------------------
		// RAMA TEMPERATURA: SELECCIÓN (Setpoint vs Histéresis)
		// ----------------------------------------------------------------
		case ST_MEN_TEMP_SELECT:
			put_cmd_task_display(CMD_DISP_TO_LINE_0, NULL);
			put_cmd_task_display(CMD_DISP_WRITE_STR, "Conf. Temp:     ");

			put_cmd_task_display(CMD_DISP_TO_LINE_1, NULL);
			if (p_task_menu_dta->current_selection == 0)      put_cmd_task_display(CMD_DISP_WRITE_STR, "> Setpoint      ");
			else if (p_task_menu_dta->current_selection == 1) put_cmd_task_display(CMD_DISP_WRITE_STR, "> Histeresis    ");

			if (true == p_task_menu_dta->flag)
			{
				p_task_menu_dta->flag = false;
				if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
				{
					p_task_menu_dta->current_selection = (p_task_menu_dta->current_selection + 1) % 2;
				} else if (EV_MEN_PRE_ACTIVE == p_task_menu_dta->event)
				{
					if (p_task_menu_dta->current_selection > 0)
						p_task_menu_dta->current_selection--;
					else
						p_task_menu_dta->current_selection = 1;
				}
				else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)
				{
					if (p_task_menu_dta->current_selection == 0) p_task_menu_dta->state = ST_MEN_MOD_TEMP_SET;
					else p_task_menu_dta->state = ST_MEN_MOD_TEMP_HYS;
				}
				else if (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
				{
					p_task_menu_dta->state = ST_MEN_MAIN_SELECT;
					p_task_menu_dta->current_selection = 0;
				}
			}
			break;

		// ----------------------------------------------------------------
		// RAMA TEMPERATURA: MODIFICAR SETPOINT
		// ----------------------------------------------------------------
		case ST_MEN_MOD_TEMP_SET:
			put_cmd_task_display(CMD_DISP_TO_LINE_0, NULL);
			put_cmd_task_display(CMD_DISP_WRITE_STR, "Temp Setpoint:  ");

			put_cmd_task_display(CMD_DISP_TO_LINE_1, NULL);
			// Muestra el valor actual que estamos editando
			snprintf(menu_str, sizeof(menu_str), "Val: %2lu \xDF""C      ", p_task_menu_dta->cfg.temp_setpoint);
			put_cmd_task_display(CMD_DISP_WRITE_STR, menu_str);

			if (true == p_task_menu_dta->flag)
			{
				p_task_menu_dta->flag = false;
				if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
				{
					p_task_menu_dta->cfg.temp_setpoint++;
					if(p_task_menu_dta->cfg.temp_setpoint > TEMP_SETPOINT_MAX) p_task_menu_dta->cfg.temp_setpoint = TEMP_SETPOINT_MIN;
				}
				else if (EV_MEN_PRE_ACTIVE == p_task_menu_dta->event)
				{
					if (p_task_menu_dta->cfg.temp_setpoint > TEMP_SETPOINT_MIN)
						p_task_menu_dta->cfg.temp_setpoint--;
					else
						p_task_menu_dta->cfg.temp_setpoint = TEMP_SETPOINT_MAX;
				}
				else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)
				{
					// Volvemos y guardamos el valor seteado
					p_task_menu_dta->state = ST_MEN_TEMP_SELECT;
					p_shared_data->cfg.temp_setpoint = p_task_menu_dta->cfg.temp_setpoint;
				}
				else if (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
				{
					// Volvemos y restauramos el valor anterior
					p_task_menu_dta->state = ST_MEN_TEMP_SELECT;
					p_task_menu_dta->cfg.temp_setpoint = p_shared_data->cfg.temp_setpoint;
				}
			}
			break;

		// ----------------------------------------------------------------
		// RAMA TEMPERATURA: MODIFICAR HISTÉRESIS
		// ----------------------------------------------------------------
		case ST_MEN_MOD_TEMP_HYS:
			put_cmd_task_display(CMD_DISP_TO_LINE_0, NULL);
			put_cmd_task_display(CMD_DISP_WRITE_STR, "Temp Histeresis:");

			put_cmd_task_display(CMD_DISP_TO_LINE_1, NULL);
			snprintf(menu_str, sizeof(menu_str), "Val: %2lu \xDF""C      ", p_task_menu_dta->cfg.temp_hysteresis);
			put_cmd_task_display(CMD_DISP_WRITE_STR, menu_str);

			if (true == p_task_menu_dta->flag)
			{
				p_task_menu_dta->flag = false;
				if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
				{
					p_task_menu_dta->cfg.temp_hysteresis++;
					if(p_task_menu_dta->cfg.temp_hysteresis > TEMP_HYSTERESIS_MAX) p_task_menu_dta->cfg.temp_hysteresis = TEMP_HYSTERESIS_MIN;
				}
				else if (EV_MEN_PRE_ACTIVE == p_task_menu_dta->event)
				{
					if (p_task_menu_dta->cfg.temp_hysteresis > TEMP_HYSTERESIS_MIN)
						p_task_menu_dta->cfg.temp_hysteresis--;
					else
						p_task_menu_dta->cfg.temp_hysteresis = TEMP_HYSTERESIS_MAX;
				}
				else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)
				{
					// Volvemos y guardamos el valor seteado
					p_task_menu_dta->state = ST_MEN_TEMP_SELECT;
					p_shared_data->cfg.temp_hysteresis = p_task_menu_dta->cfg.temp_hysteresis;
				}
				else if (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
				{
					// Volvemos y restauramos el valor anterior
					p_task_menu_dta->state = ST_MEN_TEMP_SELECT;
					p_task_menu_dta->cfg.temp_hysteresis = p_shared_data->cfg.temp_hysteresis;
				}
			}
			break;

		// ----------------------------------------------------------------
		// RAMA PRESIÓN: SELECCIÓN (Setpoint vs Histéresis)
		// ----------------------------------------------------------------
		case ST_MEN_PRESS_SELECT:
			put_cmd_task_display(CMD_DISP_TO_LINE_0, NULL);
			put_cmd_task_display(CMD_DISP_WRITE_STR, "Conf. Pres:     ");

			put_cmd_task_display(CMD_DISP_TO_LINE_1, NULL);
			if (p_task_menu_dta->current_selection == 0)      put_cmd_task_display(CMD_DISP_WRITE_STR, "> Setpoint      ");
			else if (p_task_menu_dta->current_selection == 1) put_cmd_task_display(CMD_DISP_WRITE_STR, "> Histeresis    ");

			if (true == p_task_menu_dta->flag)
			{
				p_task_menu_dta->flag = false;
				if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
				{
					p_task_menu_dta->current_selection = (p_task_menu_dta->current_selection + 1) % 2;
				}
				else if (EV_MEN_PRE_ACTIVE == p_task_menu_dta->event)
				{
					if (p_task_menu_dta->current_selection > 0)
						p_task_menu_dta->current_selection--;
					else
						p_task_menu_dta->current_selection = 1;
				}
				else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)
				{
					if (p_task_menu_dta->current_selection == 0) p_task_menu_dta->state = ST_MEN_MOD_PRESS_SET;
					else p_task_menu_dta->state = ST_MEN_MOD_PRESS_HYS;
				}
				else if (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
				{
					p_task_menu_dta->state = ST_MEN_MAIN_SELECT;
					p_task_menu_dta->current_selection = 1;
				}
			}
			break;

		// ----------------------------------------------------------------
		// RAMA PRESIÓN: MODIFICAR SETPOINT
		// ----------------------------------------------------------------
		case ST_MEN_MOD_PRESS_SET:
			put_cmd_task_display(CMD_DISP_TO_LINE_0, NULL);
			put_cmd_task_display(CMD_DISP_WRITE_STR, "Pres Setpoint:  ");

			put_cmd_task_display(CMD_DISP_TO_LINE_1, NULL);
			// Muestra el valor actual que estamos editando
			snprintf(menu_str, sizeof(menu_str), "Val: %3lu kPa   ", p_task_menu_dta->cfg.press_setpoint);
			put_cmd_task_display(CMD_DISP_WRITE_STR, menu_str);

			if (true == p_task_menu_dta->flag)
			{
				p_task_menu_dta->flag = false;
				if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
				{
					p_task_menu_dta->cfg.press_setpoint++;
					if(p_task_menu_dta->cfg.press_setpoint > PRESS_SETPOINT_MAX) p_task_menu_dta->cfg.press_setpoint = PRESS_SETPOINT_MIN;
				}
				else if (EV_MEN_PRE_ACTIVE == p_task_menu_dta->event)
				{
					if (p_task_menu_dta->cfg.press_setpoint > PRESS_SETPOINT_MIN)
						p_task_menu_dta->cfg.press_setpoint--;
					else
						p_task_menu_dta->cfg.press_setpoint = PRESS_SETPOINT_MAX;
				}
				else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)
				{
					// Volvemos y guardamos el valor seteado
					p_task_menu_dta->state = ST_MEN_PRESS_SELECT;
					p_shared_data->cfg.press_setpoint = p_task_menu_dta->cfg.press_setpoint;
				}
				else if (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
				{
					// Volvemos y restauramos el valor anterior
					p_task_menu_dta->state = ST_MEN_PRESS_SELECT;
					p_task_menu_dta->cfg.press_setpoint = p_shared_data->cfg.press_setpoint;
				}
			}
			break;

		// ----------------------------------------------------------------
		// RAMA PRESIÓN: MODIFICAR HISTÉRESIS
		// ----------------------------------------------------------------
		case ST_MEN_MOD_PRESS_HYS:
			put_cmd_task_display(CMD_DISP_TO_LINE_0, NULL);
			put_cmd_task_display(CMD_DISP_WRITE_STR, "Pres Histeresis:");

			put_cmd_task_display(CMD_DISP_TO_LINE_1, NULL);
			snprintf(menu_str, sizeof(menu_str), "Val: %3lu kPa   ", p_task_menu_dta->cfg.press_hysteresis);
			put_cmd_task_display(CMD_DISP_WRITE_STR, menu_str);

			if (true == p_task_menu_dta->flag)
			{
				p_task_menu_dta->flag = false;
				if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
				{
					p_task_menu_dta->cfg.press_hysteresis++;
					if (p_task_menu_dta->cfg.press_hysteresis > PRESS_HYSTERESIS_MAX) p_task_menu_dta->cfg.press_hysteresis = PRESS_HYSTERESIS_MIN;
				}
				else if (EV_MEN_PRE_ACTIVE == p_task_menu_dta->event)
				{
					if (p_task_menu_dta->cfg.press_hysteresis > PRESS_HYSTERESIS_MIN)
						p_task_menu_dta->cfg.press_hysteresis--;
					else
						p_task_menu_dta->cfg.press_hysteresis = PRESS_HYSTERESIS_MAX;
				}
				else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)
				{
					// Volvemos y guardamos el valor seteado
					p_task_menu_dta->state = ST_MEN_PRESS_SELECT;
					p_shared_data->cfg.press_hysteresis = p_task_menu_dta->cfg.press_hysteresis;
				}
				else if (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
				{
					// Volvemos y restauramos el valor anterior
					p_task_menu_dta->state = ST_MEN_PRESS_SELECT;
					p_task_menu_dta->cfg.press_hysteresis = p_shared_data->cfg.press_hysteresis;
				}
			}
			break;

			// ----------------------------------------------------------------
			// RAMA ALARMAS: SELECCIÓN (Habilitación, Temp, Presión)
			// ----------------------------------------------------------------
			case ST_MEN_ALARM_SELECT:
				put_cmd_task_display(CMD_DISP_TO_LINE_0, NULL);
				put_cmd_task_display(CMD_DISP_WRITE_STR, "Conf. Alarmas:  ");

				put_cmd_task_display(CMD_DISP_TO_LINE_1, NULL);
				if (p_task_menu_dta->current_selection == 0)      put_cmd_task_display(CMD_DISP_WRITE_STR, "> Habilitacion  ");
				else if (p_task_menu_dta->current_selection == 1) put_cmd_task_display(CMD_DISP_WRITE_STR, "> Limite Temp.  ");
				else if (p_task_menu_dta->current_selection == 2) put_cmd_task_display(CMD_DISP_WRITE_STR, "> Limite Pres.  ");

				if (true == p_task_menu_dta->flag)
				{
					p_task_menu_dta->flag = false;
					if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
					{
						p_task_menu_dta->current_selection = (p_task_menu_dta->current_selection + 1) % 3;
					}
					else if (EV_MEN_PRE_ACTIVE == p_task_menu_dta->event)
					{
						if (p_task_menu_dta->current_selection > 0)
							p_task_menu_dta->current_selection--;
						else
							p_task_menu_dta->current_selection = 2;
					}
					else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)
					{
						if (p_task_menu_dta->current_selection == 0)      p_task_menu_dta->state = ST_MEN_MOD_ALARM_EN;
						else if (p_task_menu_dta->current_selection == 1) p_task_menu_dta->state = ST_MEN_MOD_ALARM_TLIM;
						else if (p_task_menu_dta->current_selection == 2) p_task_menu_dta->state = ST_MEN_MOD_ALARM_PLIM;
					}
					else if (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
					{
						p_task_menu_dta->state = ST_MEN_MAIN_SELECT;
						p_task_menu_dta->current_selection = 2;
					}
				}
				break;

		// ----------------------------------------------------------------
		// RAMA ALARMAS: HABILITACIÓN
		// ----------------------------------------------------------------
		case ST_MEN_MOD_ALARM_EN:
			put_cmd_task_display(CMD_DISP_TO_LINE_0, NULL);
			put_cmd_task_display(CMD_DISP_WRITE_STR, "Alarma activa?  ");

			put_cmd_task_display(CMD_DISP_TO_LINE_1, NULL);
			if (p_task_menu_dta->cfg.alarm_enable) put_cmd_task_display(CMD_DISP_WRITE_STR, "> SI            \n");
			else                                   put_cmd_task_display(CMD_DISP_WRITE_STR, "> NO            \n");

			if (true == p_task_menu_dta->flag)
			{
				p_task_menu_dta->flag = false;
				if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
				{
					// Toggle bool
					p_task_menu_dta->cfg.alarm_enable = !p_task_menu_dta->cfg.alarm_enable;
				}
				else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)
				{
					// Volvemos y guardamos el valor seteado
					p_task_menu_dta->state = ST_MEN_ALARM_SELECT;
					p_shared_data->cfg.alarm_enable = p_task_menu_dta->cfg.alarm_enable;
				}
				else if (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
				{
					// Volvemos y restauramos el valor anterior
					p_task_menu_dta->state = ST_MEN_ALARM_SELECT;
					p_task_menu_dta->cfg.alarm_enable = p_shared_data->cfg.alarm_enable;
				}
			}
			break;

			// ----------------------------------------------------------------
			// RAMA ALARMAS: MODIFICAR LÍMITE DE TEMP
			// ----------------------------------------------------------------
			case ST_MEN_MOD_ALARM_TLIM:
				put_cmd_task_display(CMD_DISP_TO_LINE_0, NULL);
				put_cmd_task_display(CMD_DISP_WRITE_STR, "Limite Temp:    ");

				put_cmd_task_display(CMD_DISP_TO_LINE_1, NULL);
				snprintf(menu_str, sizeof(menu_str), "Val: %2lu \xDF""C      ", p_task_menu_dta->cfg.temp_alarm_limit);
				put_cmd_task_display(CMD_DISP_WRITE_STR, menu_str);

				if (true == p_task_menu_dta->flag)
				{
					p_task_menu_dta->flag = false;
					if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
					{
						p_task_menu_dta->cfg.temp_alarm_limit++;
						if (p_task_menu_dta->cfg.temp_alarm_limit > TEMP_SETPOINT_MAX) p_task_menu_dta->cfg.temp_alarm_limit = TEMP_SETPOINT_MIN;
					}
					else if (EV_MEN_PRE_ACTIVE == p_task_menu_dta->event)
					{
						if (p_task_menu_dta->cfg.temp_alarm_limit > TEMP_SETPOINT_MIN)
							p_task_menu_dta->cfg.temp_alarm_limit--;
						else
							p_task_menu_dta->cfg.temp_alarm_limit = TEMP_SETPOINT_MAX;
					}
					else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)
					{
						// Volvemos y guardamos el valor seteado
						p_task_menu_dta->state = ST_MEN_ALARM_SELECT;
						p_shared_data->cfg.temp_alarm_limit = p_task_menu_dta->cfg.temp_alarm_limit;
					}
					else if (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
					{
						// Volvemos y restauramos el valor anterior
						p_task_menu_dta->state = ST_MEN_ALARM_SELECT;
						p_task_menu_dta->cfg.temp_alarm_limit = p_shared_data->cfg.temp_alarm_limit;
					}
				}
				break;
				// ----------------------------------------------------------------
				// RAMA ALARMAS: MODIFICAR LÍMITE DE PRES
				// ----------------------------------------------------------------
				case ST_MEN_MOD_ALARM_PLIM:
					put_cmd_task_display(CMD_DISP_TO_LINE_0, NULL);
					put_cmd_task_display(CMD_DISP_WRITE_STR, "Limite Pres:    ");

					put_cmd_task_display(CMD_DISP_TO_LINE_1, NULL);
					snprintf(menu_str, sizeof(menu_str), "Val: %3lu kPa   ", p_task_menu_dta->cfg.press_alarm_limit);
					put_cmd_task_display(CMD_DISP_WRITE_STR, menu_str);

					if (true == p_task_menu_dta->flag)
					{
						p_task_menu_dta->flag = false;
						if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
						{
							p_task_menu_dta->cfg.press_alarm_limit++;
							if (p_task_menu_dta->cfg.press_alarm_limit > PRESS_SETPOINT_MAX) p_task_menu_dta->cfg.press_alarm_limit = PRESS_SETPOINT_MIN;
						}
						else if (EV_MEN_PRE_ACTIVE == p_task_menu_dta->event)
						{
							if (p_task_menu_dta->cfg.press_alarm_limit > PRESS_SETPOINT_MIN)
								p_task_menu_dta->cfg.press_alarm_limit--;
							else
								p_task_menu_dta->cfg.press_alarm_limit = PRESS_SETPOINT_MIN;
						}
						else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)
						{
							// Volvemos y guardamos el valor seteado
							p_task_menu_dta->state = ST_MEN_ALARM_SELECT;
							p_shared_data->cfg.press_alarm_limit = p_task_menu_dta->cfg.press_alarm_limit;
						}
						else if (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
						{
							// Volvemos y restauramos el valor anterior
							p_task_menu_dta->state = ST_MEN_ALARM_SELECT;
							p_task_menu_dta->cfg.press_alarm_limit = p_shared_data->cfg.press_alarm_limit;
						}
					}
					break;

		default:

			p_task_menu_dta->tick  = DEL_MEN_XX_MAX;
			p_task_menu_dta->state = ST_MEN_IDLE_VIEW;
			p_task_menu_dta->event = EV_MEN_ENT_IDLE;
			p_task_menu_dta->flag  = false;

			break;
		}
	}
}

void recover_saved_cfg(system_config_t *cfg)
{
	cfg->temp_setpoint = TEMP_SETPOINT_INI;
	cfg->temp_hysteresis = TEMP_HYSTERESIS_INI;
	cfg->press_setpoint = PRESS_SETPOINT_INI;
	cfg->press_hysteresis = PRESS_HYSTERESIS_INI;
	cfg->alarm_enable = ALARM_ENABLE_INI;

	system_config_t saved_cfg = {0};
	eeprom_read(MENU_CFG_ADDR, (void*)&saved_cfg, sizeof(saved_cfg));

	// Solo usamos los datos de la EEPROM si tienen valores razonables. Si no, dejamos el valor por omisión.

	// Temperatura
	if (is_in_range(saved_cfg.temp_setpoint, TEMP_SETPOINT_MIN, TEMP_SETPOINT_MAX))
		cfg->temp_setpoint = saved_cfg.temp_setpoint;

	if (is_in_range(saved_cfg.temp_hysteresis, TEMP_HYSTERESIS_MIN, TEMP_HYSTERESIS_MAX))
		cfg->temp_hysteresis = saved_cfg.temp_hysteresis;

	if (is_in_range(saved_cfg.temp_alarm_limit, TEMP_SETPOINT_MIN, TEMP_SETPOINT_MAX))
		cfg->temp_alarm_limit = saved_cfg.temp_alarm_limit;

	// Presión
	if (is_in_range(saved_cfg.press_setpoint, PRESS_SETPOINT_MIN, PRESS_SETPOINT_MAX))
		cfg->press_setpoint = saved_cfg.press_setpoint;

	if (is_in_range(saved_cfg.press_hysteresis, PRESS_HYSTERESIS_MIN, PRESS_HYSTERESIS_MAX))
		cfg->press_hysteresis = saved_cfg.press_hysteresis;

	if (is_in_range(saved_cfg.press_alarm_limit, PRESS_SETPOINT_MIN, PRESS_SETPOINT_MAX))
		cfg->press_alarm_limit = saved_cfg.press_alarm_limit;

	// Alarma
	cfg->alarm_enable = saved_cfg.alarm_enable;
}

/********************** end of file ******************************************/
