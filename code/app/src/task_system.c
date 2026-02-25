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
 * @file   : task_system.c
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
#include "task_system_attribute.h"
#include "task_system_interface.h"
#include "task_menu_interface.h"
#include "task_menu_attribute.h"
#include "task_actuator_attribute.h"
#include "task_actuator_interface.h"
#include "task_temp_interface.h"
#include "task_press_interface.h"
#include "task_display_interface.h"
#include "utils.h"

#include <stdbool.h>

/********************** macros and definitions *******************************/
#define G_TASK_SYS_CNT_INI			0ul
#define G_TASK_SYS_TICK_CNT_INI		0ul

#define DEL_SYS_XX_MIN				0ul
#define DEL_SYS_XX_MED				50ul
#define DEL_SYS_XX_MAX				500ul

/********************** internal data declaration ****************************/
task_system_dta_t task_system_dta =
	{DEL_SYS_XX_MIN, ST_SYS_MENU_MODE, EV_SYS_ENABLE_IDLE, false, false};

#define SYSTEM_DTA_QTY	(sizeof(task_system_dta)/sizeof(task_system_dta_t))

static char system_str[17];

/********************** internal functions declaration ***********************/

static bool is_menu_button_event(task_system_ev_t event);
static task_menu_ev_t system_event_to_menu_event(task_system_ev_t system_ev);
static void task_system_statechart(shared_data_type *p_shared_data);

/********************** internal data definition *****************************/
const char *p_task_system 		= "Task System (System Statechart)";
const char *p_task_system_ 		= "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_system_cnt;
volatile uint32_t g_task_system_tick_cnt;

/********************** external functions definition ************************/
void task_system_init(void *parameters)
{
	task_system_dta_t 	*p_task_system_dta;
	task_system_st_t	state;
	task_system_ev_t	event;
	bool b_event;

	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_system_init), p_task_system);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_system), p_task_system_);

	g_task_system_cnt = G_TASK_SYS_CNT_INI;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_system_cnt), g_task_system_cnt);

	init_queue_event_task_system();

	/* Update Task Actuator Configuration & Data Pointer */
	p_task_system_dta = &task_system_dta;

	/* Print out: Task execution FSM */
	state = p_task_system_dta->state;
	LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

	event = p_task_system_dta->event;
	LOGGER_LOG("   %s = %lu", GET_NAME(event), (uint32_t)event);

	b_event = p_task_system_dta->flag;
	LOGGER_LOG("   %s = %s\r\n", GET_NAME(b_event), (b_event ? "true" : "false"));

	g_task_system_tick_cnt = G_TASK_SYS_TICK_CNT_INI;
}

void task_system_update(void *parameters)
{
	bool b_time_update_required = false;

	/* Update Task System Counter */
	g_task_system_cnt++;

	/* Protect shared resource (g_task_system_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_SYS_TICK_CNT_INI < g_task_system_tick_cnt)
    {
    	g_task_system_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
		/* Protect shared resource (g_task_system_tick) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_SYS_TICK_CNT_INI < g_task_system_tick_cnt)
		{
			g_task_system_tick_cnt--;
			b_time_update_required = true;
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

		task_system_statechart((shared_data_type *)parameters);
	}
}

static void task_system_statechart(shared_data_type *p_shared_data) {
	task_system_dta_t *p_task_system_dta;

	bool b_display_update_required = false;

	uint32_t temp = temp_raw_to_celsius(p_shared_data->temp_raw);
	uint32_t press = press_raw_to_kPa(p_shared_data->pressure_raw);
	bool b_is_alarm_set = false;

	/* Update Task System Data Pointer */
	p_task_system_dta = &task_system_dta;

	if (DEL_SYS_XX_MIN < p_task_system_dta->tick)
	{
		p_task_system_dta->tick--;
	}
	else
	{
		p_task_system_dta->tick = DEL_SYS_XX_MAX;
		b_display_update_required = true;
	}

	if (true == any_event_task_system())
	{
		p_task_system_dta->flag = true;
		p_task_system_dta->event = get_event_task_system();
	}

	switch (p_task_system_dta->state)
	{
	case ST_SYS_MENU_MODE:
		if ((true == p_task_system_dta->flag)
				&& is_menu_button_event(p_task_system_dta->event))
		{
			p_task_system_dta->flag = false;
			task_menu_ev_t menu_ev = system_event_to_menu_event(p_task_system_dta->event);
			put_event_task_menu(menu_ev);
		}
		else if ((true == p_task_system_dta->flag)
				&& (EV_SYS_ENABLE_ACTIVE == p_task_system_dta->event))
		{
			p_task_system_dta->flag = false;
			p_task_system_dta->enabled = true;
		}
		else if ((true == p_task_system_dta->flag)
				&& (EV_SYS_ENABLE_IDLE == p_task_system_dta->event))
		{
			p_task_system_dta->flag = false;
			p_task_system_dta->enabled = false;
		}
		else if ((true == p_task_system_dta->flag)
				&& (EV_SYS_EXIT_MENU == p_task_system_dta->event))
		{
			p_task_system_dta->flag = false;
			p_task_system_dta->state = ST_SYS_NORMAL_MODE;
		}

		break;

	case ST_SYS_NORMAL_MODE:
		if (b_display_update_required)
		{
			put_cmd_task_display(CMD_DISP_TO_LINE_0, NULL);

			build_status_bar(system_str, temp, press);

			put_cmd_task_display(CMD_DISP_WRITE_STR, system_str);
			put_cmd_task_display(CMD_DISP_TO_LINE_1, NULL);
			put_cmd_task_display(CMD_DISP_WRITE_STR, "Estado: ");
			put_cmd_task_display(CMD_DISP_WRITE_STR,
					(p_task_system_dta->enabled) ? "on      " : "off     ");
		}

		if (p_task_system_dta->enabled && p_shared_data->cfg.alarm_enabled)
		{
			if (p_shared_data->cfg.temp_alarm_limit
					> p_shared_data->cfg.temp_setpoint)
				b_is_alarm_set |= (temp > p_shared_data->cfg.temp_alarm_limit);
			else
				b_is_alarm_set |= (temp < p_shared_data->cfg.temp_alarm_limit);

			if (p_shared_data->cfg.press_alarm_limit
					> p_shared_data->cfg.press_setpoint)
				b_is_alarm_set |=
						(press > p_shared_data->cfg.press_alarm_limit);
			else
				b_is_alarm_set |=
						(press < p_shared_data->cfg.press_alarm_limit);
		}

		if (b_is_alarm_set)
		{
			p_task_system_dta->state = ST_SYS_ALARM_MODE;
			put_event_task_actuator(EV_ACT_XX_BLINK, ID_ACT_BUZZER);
		}
		else if ((true == p_task_system_dta->flag)
				&& (EV_SYS_ENT_ACTIVE == p_task_system_dta->event))
		{
			p_task_system_dta->flag = false;
			p_task_system_dta->state = ST_SYS_MENU_MODE;
			put_event_task_menu(EV_MEN_ENT_ACTIVE);
		}
		else if ((true == p_task_system_dta->flag)
				&& (EV_SYS_ENABLE_ACTIVE == p_task_system_dta->event))
		{
			p_task_system_dta->flag = false;
			p_task_system_dta->enabled = true;
			put_event_task_temp(EV_TEMP_ENABLE_ON);
			put_event_task_press(EV_PRESS_ENABLE_ON);
		}
		else if ((true == p_task_system_dta->flag)
				&& (EV_SYS_ENABLE_IDLE == p_task_system_dta->event))
		{
			p_task_system_dta->flag = false;
			p_task_system_dta->enabled = false;
			put_event_task_temp(EV_TEMP_ENABLE_OFF);
			put_event_task_press(EV_PRESS_ENABLE_OFF);
		}

		break;

	case ST_SYS_ALARM_MODE:
		if (b_display_update_required)
		{
			put_cmd_task_display(CMD_DISP_TO_LINE_0, NULL);
			build_status_bar(system_str, temp, press);
			put_cmd_task_display(CMD_DISP_WRITE_STR, system_str);
			put_cmd_task_display(CMD_DISP_TO_LINE_1, NULL);
			put_cmd_task_display(CMD_DISP_WRITE_STR, "    ALARMA!     ");
		}
		if ((true == p_task_system_dta->flag)
				&& (EV_SYS_ENABLE_IDLE == p_task_system_dta->event))
		{
			p_task_system_dta->flag = false;
			p_task_system_dta->enabled = false;
			p_task_system_dta->state = ST_SYS_NORMAL_MODE;
			put_event_task_temp(EV_TEMP_ENABLE_OFF);
			put_event_task_press(EV_PRESS_ENABLE_OFF);
			put_event_task_actuator(EV_ACT_XX_NOT_BLINK, ID_ACT_BUZZER);
		}
		break;

	default:
		break;
	}
}

static bool is_menu_button_event(task_system_ev_t event)
{
	switch (event)
	{
	case EV_SYS_ENT_IDLE:
    case EV_SYS_ENT_ACTIVE:
	case EV_SYS_NEX_IDLE:
	case EV_SYS_NEX_ACTIVE:
	case EV_SYS_PRE_IDLE:
	case EV_SYS_PRE_ACTIVE:
	case EV_SYS_ESC_IDLE:
	case EV_SYS_ESC_ACTIVE:
		return true;
	default:
		return false;
	}
}

static task_menu_ev_t system_event_to_menu_event(task_system_ev_t system_ev)
{
	task_menu_ev_t menu_ev;
	switch (system_ev)
	{
	case EV_SYS_ENT_IDLE:
		menu_ev = EV_MEN_ENT_IDLE;
		break;
	case EV_SYS_ENT_ACTIVE:
		menu_ev = EV_MEN_ENT_ACTIVE;
		break;
	case EV_SYS_NEX_IDLE:
		menu_ev = EV_MEN_NEX_IDLE;
		break;
	case EV_SYS_NEX_ACTIVE:
		menu_ev = EV_MEN_NEX_ACTIVE;
		break;
	case EV_SYS_PRE_IDLE:
		menu_ev = EV_MEN_PRE_IDLE;
		break;
	case EV_SYS_PRE_ACTIVE:
		menu_ev = EV_MEN_PRE_ACTIVE;
		break;
	case EV_SYS_ESC_IDLE:
		menu_ev = EV_MEN_ESC_IDLE;
		break;
	case EV_SYS_ESC_ACTIVE:
		menu_ev = EV_MEN_ESC_ACTIVE;
		break;
	default:
		// No debería ocurrir
		__builtin_unreachable();
		while (1) {}
		break;
	}
	return menu_ev;
}

/********************** end of file ******************************************/
