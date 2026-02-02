/*
 * @file   : task_press.c
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
#include "task_press_attribute.h"
#include "task_press_interface.h"
#include "task_actuator_attribute.h"
#include "task_actuator_interface.h"
#include "utils.h"

#include <stdbool.h>

/********************** macros and definitions *******************************/
#define G_TASK_PRESS_CNT_INI		0ul
#define G_TASK_PRESS_TICK_CNT_INI	0ul


/********************** internal data declaration ****************************/
task_press_dta_t task_press_dta =
	{ST_PRESS_OFF, EV_PRESS_ENABLE_OFF, false};

#define PRESS_DTA_QTY	(sizeof(task_press_dta)/sizeof(task_press_dta_t))

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_press 		= "Task Press (Pressure control)";
const char *p_task_press_ 		= "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_press_cnt;
volatile uint32_t g_task_press_tick_cnt;

/********************** external functions definition ************************/
void task_press_init(void *parameters)
{
	task_press_dta_t 	*p_task_press_dta;
	task_press_st_t	state;
	task_press_ev_t	event;
	bool b_event;

	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_press_init), p_task_press);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_press), p_task_press_);

	g_task_press_cnt = G_TASK_PRESS_CNT_INI;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_press_cnt), g_task_press_cnt);

	init_queue_event_task_press();

	/* Update Task Actuator Configuration & Data Pointer */
	p_task_press_dta = &task_press_dta;

	/* Print out: Task execution FSM */
	state = p_task_press_dta->state;
	LOGGER_LOG("   %s = %lu", GET_NAME(state), (uint32_t)state);

	event = p_task_press_dta->event;
	LOGGER_LOG("   %s = %lu", GET_NAME(event), (uint32_t)event);

	b_event = p_task_press_dta->flag;
	LOGGER_LOG("   %s = %s\r\n", GET_NAME(b_event), (b_event ? "true" : "false"));

	g_task_press_tick_cnt = G_TASK_PRESS_TICK_CNT_INI;
}

void task_press_update(void *parameters)
{
	shared_data_type *shared_data = (shared_data_type*)parameters;

	task_press_dta_t *p_task_press_dta;
	bool b_time_update_required = false;

	uint32_t press = press_raw_to_kPa(shared_data->pressure_raw);

	/* Update Task System Counter */
	g_task_press_cnt++;

	/* Protect shared resource (g_task_press_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_PRESS_TICK_CNT_INI < g_task_press_tick_cnt)
    {
    	g_task_press_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
		/* Protect shared resource (g_task_press_tick) */
		__asm("CPSID i");	/* disable interrupts*/
		if (G_TASK_PRESS_TICK_CNT_INI < g_task_press_tick_cnt)
		{
			g_task_press_tick_cnt--;
			b_time_update_required = true;
		}
		else
		{
			b_time_update_required = false;
		}
		__asm("CPSIE i");	/* enable interrupts*/

    	/* Update Task Press Data Pointer */
		p_task_press_dta = &task_press_dta;

		if (true == any_event_task_press())
		{
			p_task_press_dta->flag = true;
			p_task_press_dta->event = get_event_task_press();
		}

		switch (p_task_press_dta->state)
		{
		case ST_PRESS_OFF:
			if ((true == p_task_press_dta->flag) && (EV_PRESS_ENABLE_ON == p_task_press_dta->event))
			{
				p_task_press_dta->flag = false;
				p_task_press_dta->state = ST_PRESS_IDLE;
			}
			break;

		case ST_PRESS_IDLE:
			if ((true == p_task_press_dta->flag) && (EV_PRESS_ENABLE_OFF == p_task_press_dta->event))
			{
				p_task_press_dta->flag = false;
				p_task_press_dta->state = ST_PRESS_OFF;
				// TODO: Abrir válvula (no queremos mantener vacío si apagamos el control)
			}
			// Equivalente a (press < setpoint - hist) pero evita underflow si (hist > setpoint)
			else if (press + shared_data->cfg.press_hysteresis < shared_data->cfg.press_setpoint)
			{
				p_task_press_dta->state = ST_PRESS_RELEASE;
				// TODO: Abrir válvula
			}
			else if (press > shared_data->cfg.press_setpoint + shared_data->cfg.press_hysteresis)
			{
				p_task_press_dta->state = ST_PRESS_VACUUM;
				// TODO: Encender la bomba de vacío
			}
			break;

		case ST_PRESS_RELEASE:
			if ((true == p_task_press_dta->flag) && (EV_PRESS_ENABLE_OFF == p_task_press_dta->event))
			{
				p_task_press_dta->flag = false;
				p_task_press_dta->state = ST_PRESS_OFF;
				// No hace falta cerrar la válvula, tiene que quedar abierta
			}
			else if (press > shared_data->cfg.press_setpoint)
			{
				p_task_press_dta->state = ST_PRESS_IDLE;
				// TODO: Cerrar la válvula
			}
			break;

		case ST_PRESS_VACUUM:
			if ((true == p_task_press_dta->flag) && (EV_PRESS_ENABLE_OFF == p_task_press_dta->event))
			{
				p_task_press_dta->flag = false;
				p_task_press_dta->state = ST_PRESS_OFF;
				// TODO: Apagar bomba y abrir válvula
			}
			else if (press < shared_data->cfg.press_setpoint)
			{
				p_task_press_dta->state = ST_PRESS_IDLE;
				// TODO: Apagar la bomba
			}
			break;

		default:
			break;
		}
	}
}

/********************** end of file ******************************************/
