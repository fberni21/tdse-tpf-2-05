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
#include "display.h"

/********************** macros and definitions *******************************/
#define G_TASK_MEN_CNT_INI			0ul
#define G_TASK_MEN_TICK_CNT_INI		0ul

#define DEL_MEN_XX_MIN				0ul
#define DEL_MEN_XX_MED				50ul
#define DEL_MEN_XX_MAX				500ul


#define TEMP_SETPOINT_INI		25 		// celsius
#define TEMP_HYSTERESIS_INI		2 		// celsius
#define PRESS_SETPOINT_INI		1013 	// hPa
#define PRESS_HYSTERESIS_INI	10 		// hPa
#define ALARM_ENABLE_INI 		true

#define TEMP_SETPOINT_MAX		80		// celsius
#define TEMP_HYSTERESIS_MAX		10		// celsius
#define PRESS_SETPOINT_MAX		1100	// hPa
#define PRESS_HYSTERESIS_MAX	100		// hPa


/********************** internal data declaration ****************************/
task_menu_dta_t task_menu_dta =
	{DEL_MEN_XX_MIN, ST_MEN_IDLE_VIEW, EV_MEN_ENT_IDLE, false, {0}, 0};

#define MENU_DTA_QTY	(sizeof(task_menu_dta)/sizeof(task_menu_dta_t))

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_menu 		= "Task Menu (Interactive Menu)";
const char *p_task_menu_ 		= "Non-Blocking & Update By Time Code";

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

	cycle_counter_init();
	cycle_counter_reset();

	//displayInit( DISPLAY_CONNECTION_GPIO_4BITS );

	task_menu_dta.cfg.temp_setpoint = TEMP_SETPOINT_INI;
	task_menu_dta.cfg.temp_hysteresis = TEMP_HYSTERESIS_INI;
	task_menu_dta.cfg.press_setpoint = PRESS_SETPOINT_INI;
	task_menu_dta.cfg.press_hysteresis = PRESS_HYSTERESIS_INI;
	task_menu_dta.cfg.alarm_enable = ALARM_ENABLE_INI;

	g_task_menu_tick_cnt = G_TASK_MEN_TICK_CNT_INI;
}

void task_menu_update(void *parameters)
{
	task_menu_dta_t *p_task_menu_dta;
	bool b_time_update_required = false;
	char menu_str[17];

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

    	/* Update Task Menu Data Pointer */
		p_task_menu_dta = &task_menu_dta;

    	if (DEL_MEN_XX_MIN < p_task_menu_dta->tick)
		{
			p_task_menu_dta->tick--;
		}
		else
		{
			p_task_menu_dta->tick = DEL_MEN_XX_MED;

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
			        // Aquí deberías leer shared_data para mostrar valores reales
			        // Ejemplo simulado:
			        //displayCharPositionWrite(0, 0);
			        snprintf(menu_str, sizeof(menu_str), "T:%luC P:%luhPa ",
			                 p_task_menu_dta->cfg.temp_setpoint, // Aquí iría temp_actual
			                 p_task_menu_dta->cfg.press_setpoint); // Aquí iría press_actual
			        printf("%s\n", menu_str);
			        //displayStringWrite(menu_str);

			        //displayCharPositionWrite(0, 1);
			        //displayStringWrite("ENT p/ Config   ");
			        printf("ENT p/ Config   \n");

			        // Si presiona ENTER, va al menú principal
			        if ((true == p_task_menu_dta->flag) && (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event))
			        {
			            p_task_menu_dta->flag = false;
			            p_task_menu_dta->state = ST_MEN_MAIN_SELECT;
			            p_task_menu_dta->current_selection = 0; // Reset selección
			        }
			        break;

				// ----------------------------------------------------------------
				// ESTADO 2: SELECCIÓN PRINCIPAL (Temp / Presion / Alarma)
				// ----------------------------------------------------------------
				case ST_MEN_MAIN_SELECT:
					//displayCharPositionWrite(0, 0);
					//displayStringWrite("Configurar:     ");
					printf("Configurar:     \n");

					//displayCharPositionWrite(0, 1);
					if (p_task_menu_dta->current_selection == 0)      printf("> Temperatura   \n");
					else if (p_task_menu_dta->current_selection == 1) printf("> Presion       \n");
					else if (p_task_menu_dta->current_selection == 2) printf("> Alarmas       \n");

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
							// Entrar a la rama correspondiente
							if (p_task_menu_dta->current_selection == 0) p_task_menu_dta->state = ST_MEN_TEMP_SELECT;
							else if (p_task_menu_dta->current_selection == 1) p_task_menu_dta->state = ST_MEN_PRESS_SELECT;
							else p_task_menu_dta->state = ST_MEN_ALARM_SELECT;

							p_task_menu_dta->current_selection = 0; // Reset para el submenú
						}
						else if (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
						{
							p_task_menu_dta->state = ST_MEN_IDLE_VIEW;
						}
					}
					break;

				// ----------------------------------------------------------------
				// RAMA TEMPERATURA: SELECCIÓN (Setpoint vs Histéresis)
				// ----------------------------------------------------------------
				case ST_MEN_TEMP_SELECT:
					//displayCharPositionWrite(0, 0);
					//displayStringWrite("Conf. Temp:     ");
					printf("Conf. Temp:     \n");

					//displayCharPositionWrite(0, 1);
					if (p_task_menu_dta->current_selection == 0)      printf("> Setpoint      \n");
					else if (p_task_menu_dta->current_selection == 1) printf("> Histeresis    \n");

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
							p_task_menu_dta->state = ST_MEN_MAIN_SELECT; // Volver atrás
							p_task_menu_dta->current_selection = 0;
						}
					}
					break;

				// ----------------------------------------------------------------
				// RAMA TEMPERATURA: MODIFICAR SETPOINT
				// ----------------------------------------------------------------
				case ST_MEN_MOD_TEMP_SET:
					//displayCharPositionWrite(0, 0);
					//displayStringWrite("Temp Setpoint:  ");
					printf("Temp Setpoint:  \n");

					//displayCharPositionWrite(0, 1);
					// Muestra el valor actual que estamos editando
					snprintf(menu_str, sizeof(menu_str), "Val: %lu C       ", p_task_menu_dta->cfg.temp_setpoint);
					printf("%s\n", menu_str);

					if (true == p_task_menu_dta->flag)
					{
						p_task_menu_dta->flag = false;
						if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
						{
							p_task_menu_dta->cfg.temp_setpoint++;
							if(p_task_menu_dta->cfg.temp_setpoint > TEMP_SETPOINT_MAX) p_task_menu_dta->cfg.temp_setpoint = 0;
						}
						else if (EV_MEN_PRE_ACTIVE == p_task_menu_dta->event)
						{
							if (p_task_menu_dta->cfg.temp_setpoint > 0)
								p_task_menu_dta->cfg.temp_setpoint--;
							else
								p_task_menu_dta->cfg.temp_setpoint = TEMP_SETPOINT_MAX;
						}
						else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)
						{
							// Confirmar y volver
							p_task_menu_dta->state = ST_MEN_TEMP_SELECT;
						}
						else if (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
						{
							// TODO: Cancelar y volver (opcionalmente podrías restaurar el valor viejo)
							p_task_menu_dta->state = ST_MEN_TEMP_SELECT;
						}
					}
					break;

				// ----------------------------------------------------------------
				// RAMA TEMPERATURA: MODIFICAR HISTÉRESIS
				// ----------------------------------------------------------------
				case ST_MEN_MOD_TEMP_HYS:
					//displayCharPositionWrite(0, 0);
					//displayStringWrite("Temp Histeresis:");
					printf("Temp Histeresis:\n");

					//displayCharPositionWrite(0, 1);
					snprintf(menu_str, sizeof(menu_str), "Val: %lu C       ", p_task_menu_dta->cfg.temp_hysteresis);
					printf("%s\n", menu_str);

					if (true == p_task_menu_dta->flag)
					{
						p_task_menu_dta->flag = false;
						if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
						{
							p_task_menu_dta->cfg.temp_hysteresis++;
							if(p_task_menu_dta->cfg.temp_hysteresis > TEMP_HYSTERESIS_MAX) p_task_menu_dta->cfg.temp_hysteresis = 1;
						}
						else if (EV_MEN_PRE_ACTIVE == p_task_menu_dta->event)
						{
							if (p_task_menu_dta->cfg.temp_hysteresis > 0)
								p_task_menu_dta->cfg.temp_hysteresis--;
							else
								p_task_menu_dta->cfg.temp_hysteresis = TEMP_HYSTERESIS_MAX;
						}
						else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event || EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
						{
							p_task_menu_dta->state = ST_MEN_TEMP_SELECT;
						}
						// TODO: lógica de escape
					}
					break;

					// ----------------------------------------------------------------
					// RAMA PRESIÓN: SELECCIÓN (Setpoint vs Histéresis)
					// ----------------------------------------------------------------
					case ST_MEN_PRESS_SELECT:
						//displayCharPositionWrite(0, 0);
						//displayStringWrite("Conf. Pres:     ");
						printf("Conf. Pres:     \n");

						//displayCharPositionWrite(0, 1);
						if (p_task_menu_dta->current_selection == 0)      printf("> Setpoint      \n");
						else if (p_task_menu_dta->current_selection == 1) printf("> Histeresis    \n");

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
									p_task_menu_dta->current_selection = 0;
							}
							else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)
							{
								if (p_task_menu_dta->current_selection == 0) p_task_menu_dta->state = ST_MEN_MOD_PRESS_SET;
								else p_task_menu_dta->state = ST_MEN_MOD_PRESS_HYS;
							}
							else if (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
							{
								p_task_menu_dta->state = ST_MEN_MAIN_SELECT; // Volver atrás
								p_task_menu_dta->current_selection = 1;
							}
						}
						break;

					// ----------------------------------------------------------------
					// RAMA PRESIÓN: MODIFICAR SETPOINT
					// ----------------------------------------------------------------
					case ST_MEN_MOD_PRESS_SET:
						//displayCharPositionWrite(0, 0);
						//displayStringWrite("Temp Setpoint:  ");

						//displayCharPositionWrite(0, 1);
						// Muestra el valor actual que estamos editando
						snprintf(menu_str, sizeof(menu_str), "Val: %lu hPa     ", p_task_menu_dta->cfg.press_setpoint);
						printf("%s\n", menu_str);

						if (true == p_task_menu_dta->flag)
						{
							p_task_menu_dta->flag = false;
							if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
							{
								p_task_menu_dta->cfg.press_setpoint += 10;
								if(p_task_menu_dta->cfg.press_setpoint > PRESS_SETPOINT_MAX) p_task_menu_dta->cfg.press_setpoint = 0;
							}
							else if (EV_MEN_PRE_ACTIVE == p_task_menu_dta->event)
							{
								if (p_task_menu_dta->cfg.press_setpoint > 0)
									p_task_menu_dta->cfg.press_setpoint -= 10;
								else
									p_task_menu_dta->cfg.press_setpoint = PRESS_SETPOINT_MAX;
							}
							else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event)
							{
								// Confirmar y volver
								p_task_menu_dta->state = ST_MEN_PRESS_SELECT;
							}
							else if (EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
							{
								// TODO: Cancelar y volver (opcionalmente podrías restaurar el valor viejo)
								p_task_menu_dta->state = ST_MEN_PRESS_SELECT;
							}
						}
						break;

					// ----------------------------------------------------------------
					// RAMA PRESIÓN: MODIFICAR HISTÉRESIS
					// ----------------------------------------------------------------
					case ST_MEN_MOD_PRESS_HYS:
						//displayCharPositionWrite(0, 0);
						//displayStringWrite("Temp Histeresis:");
						displayStringWrite("Pres Histeresis:");

						//displayCharPositionWrite(0, 1);
						snprintf(menu_str, sizeof(menu_str), "Val: %lu hPa     ", p_task_menu_dta->cfg.press_hysteresis);
						printf("%s\n", menu_str);

						if (true == p_task_menu_dta->flag)
						{
							p_task_menu_dta->flag = false;
							if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
							{
								p_task_menu_dta->cfg.press_hysteresis += 10;
								if (p_task_menu_dta->cfg.press_hysteresis > PRESS_HYSTERESIS_MAX) p_task_menu_dta->cfg.press_hysteresis = 10;
							}
							else if (EV_MEN_PRE_ACTIVE == p_task_menu_dta->event)
							{
								if (p_task_menu_dta->cfg.press_hysteresis > 10)
									p_task_menu_dta->cfg.press_hysteresis -= 10;
								else
									p_task_menu_dta->cfg.press_hysteresis = PRESS_HYSTERESIS_MAX;
							}
							else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event || EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
							{
								p_task_menu_dta->state = ST_MEN_PRESS_SELECT;
							}
						}
						break;

				// ----------------------------------------------------------------
				// RAMA ALARMAS: HABILITACIÓN
				// ----------------------------------------------------------------
				// TODO: falta la parte de select y considerar los valores de las alarmas además de si está activada
				case ST_MEN_MOD_ALARM_EN:
					 //displayCharPositionWrite(0, 0);
					 //displayStringWrite("Alarma Activa?  ");
					 printf("Alarma Activa?  \n");

					 //displayCharPositionWrite(0, 1);
					 if(p_task_menu_dta->cfg.alarm_enable) printf("> SI            \n");
					 else                                  printf("> NO            \n");

					 if (true == p_task_menu_dta->flag)
					{
						p_task_menu_dta->flag = false;
						if (EV_MEN_NEX_ACTIVE == p_task_menu_dta->event)
						{
							// Toggle bool
							p_task_menu_dta->cfg.alarm_enable = !p_task_menu_dta->cfg.alarm_enable;
						}
						else if (EV_MEN_ENT_ACTIVE == p_task_menu_dta->event || EV_MEN_ESC_ACTIVE == p_task_menu_dta->event)
						{
							p_task_menu_dta->state = ST_MEN_ALARM_SELECT;
						}
					}
					break;

				// TODO: cambiar valores de las alarmas

				default:

					p_task_menu_dta->tick  = DEL_MEN_XX_MAX;
					p_task_menu_dta->state = ST_MEN_IDLE_VIEW;
					p_task_menu_dta->event = EV_MEN_ENT_IDLE;
					p_task_menu_dta->flag  = false;

					break;
			}
		}
	}
}

/********************** end of file ******************************************/
