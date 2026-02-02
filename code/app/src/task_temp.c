/*
 * @file   : task_temp.c
 * @date   : Feb 02, 2026
 * @author : Franco Berni <fberni@fi.uba.ar>
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
#include "task_temp_attribute.h"
#include "task_temp_interface.h"
#include "task_actuator_attribute.h"
#include "task_actuator_interface.h"
#include "utils.h"

#include <stdbool.h>

/********************** macros and definitions *******************************/
#define G_TASK_TEMP_CNT_INI			0ul
#define G_TASK_TEMP_TICK_CNT_INI	0ul


/********************** internal data declaration ****************************/
task_temp_dta_t task_temp_dta =
	{ST_TEMP_OFF, EV_TEMP_ENABLE_OFF, false};

#define TEMP_DTA_QTY	(sizeof(task_temp_dta)/sizeof(task_temp_dta_t))

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_temp 		= "Task Temp (Temperature control)";
const char *p_task_temp_ 		= "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_temp_cnt;
volatile uint32_t g_task_temp_tick_cnt;

/********************** external functions definition ************************/
void task_temp_init(void *parameters)
{
	task_temp_dta_t 	*p_task_temp_dta;
	task_temp_st_t	state;
	task_temp_ev_t	event;
	bool b_event;

	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_temp_init), p_task_temp);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_temp), p_task_temp_);

	g_task_temp_cnt = G_TASK_TEMP_CNT_INI;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_temp_cnt), g_task_temp_cnt);

	init_queue_event_task_temp();

	/* Update Task Actuator Configuration & Data Pointer */
	p_task_temp_dta = &task_temp_dta;

	/* Print out: Task execution FSM */
	state = p_task_temp_dta->state;
	LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

	event = p_task_temp_dta->event;
	LOGGER_LOG("   %s = %lu", GET_NAME(event), (uint32_t)event);

	b_event = p_task_temp_dta->flag;
	LOGGER_LOG("   %s = %s\r\n", GET_NAME(b_event), (b_event ? "true" : "false"));

	g_task_temp_tick_cnt = G_TASK_TEMP_TICK_CNT_INI;
}

void task_temp_update(void *parameters)
{
	shared_data_type *shared_data = (shared_data_type*)parameters;

	task_temp_dta_t *p_task_temp_dta;
	bool b_time_update_required = false;

	uint32_t temp = temp_raw_to_celsius(shared_data->temp_raw);

	/* Update Task System Counter */
	g_task_temp_cnt++;

	/* Protect shared resource (g_task_temp_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_TEMP_TICK_CNT_INI < g_task_temp_tick_cnt)
    {
    	g_task_temp_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
		/* Protect shared resource (g_task_temp_tick) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_TEMP_TICK_CNT_INI < g_task_temp_tick_cnt)
		{
			g_task_temp_tick_cnt--;
			b_time_update_required = true;
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

    	/* Update Task System Data Pointer */
		p_task_temp_dta = &task_temp_dta;

		if (true == any_event_task_temp())
		{
			p_task_temp_dta->flag = true;
			p_task_temp_dta->event = get_event_task_temp();
		}

		switch (p_task_temp_dta->state)
		{
		case ST_TEMP_OFF:
			if ((true == p_task_temp_dta->flag) && (EV_TEMP_ENABLE_ON == p_task_temp_dta->event))
			{
				p_task_temp_dta->flag = false;
				p_task_temp_dta->state = ST_TEMP_IDLE;
			}
			break;

		case ST_TEMP_IDLE:
			if ((true == p_task_temp_dta->flag) && (EV_TEMP_ENABLE_OFF == p_task_temp_dta->event))
			{
				p_task_temp_dta->flag = false;
				p_task_temp_dta->state = ST_TEMP_OFF;
			}
			// Equivalente a (temp < setpoint - hist) pero evita underflow si (hist > setpoint)
			else if (temp + shared_data->cfg.temp_hysteresis < shared_data->cfg.temp_setpoint)
			{
				p_task_temp_dta->state = ST_TEMP_HEATING;
				// TODO: Prender el actuador del calentador
			}
			else if (temp > shared_data->cfg.temp_setpoint + shared_data->cfg.temp_hysteresis)
			{
				p_task_temp_dta->state = ST_TEMP_COOLING;
				// TODO: Prender el actuador del enfriador
			}
			break;

		case ST_TEMP_HEATING:
			if ((true == p_task_temp_dta->flag) && (EV_TEMP_ENABLE_OFF == p_task_temp_dta->event))
			{
				p_task_temp_dta->flag = false;
				p_task_temp_dta->state = ST_TEMP_OFF;
				// TODO: apagar el calentador
			}
			else if (temp > shared_data->cfg.temp_setpoint)
			{
				p_task_temp_dta->state = ST_TEMP_IDLE;
				// TODO:Apagar el actuador del calentador
			}
			break;

		case ST_TEMP_COOLING:
			if ((true == p_task_temp_dta->flag) && (EV_TEMP_ENABLE_OFF == p_task_temp_dta->event))
			{
				p_task_temp_dta->flag = false;
				p_task_temp_dta->state = ST_TEMP_OFF;
				// TODO: apagar el enfriador
			}
			else if (temp < shared_data->cfg.temp_setpoint)
			{
				p_task_temp_dta->state = ST_TEMP_IDLE;
				// TODO:Apagar el actuador del enfriador
			}
			break;

		default:
			break;
		}
	}
}

/********************** end of file ******************************************/
