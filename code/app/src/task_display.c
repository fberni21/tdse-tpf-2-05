/*
 * @file   : task_display.c
 * @date   : Jan 26, 2026
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
#include "task_display_interface.h"
#include "lcd/I2C_LCD.h"

/********************** macros and definitions *******************************/
#define G_TASK_DISPLAY_CNT_INI			0ul
#define G_TASK_DISPLAY_TICK_CNT_INI		0ul


/********************** internal data declaration ****************************/
/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_display 		= "Task display (Interactive display)";
const char *p_task_display_ 		= "Non-Blocking & Update By Time Code";

/********************** external data declaration ****************************/
uint32_t g_task_display_cnt;
volatile uint32_t g_task_display_tick_cnt;

extern I2C_HandleTypeDef hi2c2;

/********************** external functions definition ************************/
void task_display_init(void *parameters)
{
	/* Print out: Task Initialized */
	LOGGER_LOG("  %s is running - %s\r\n", GET_NAME(task_display_init), p_task_display);
	LOGGER_LOG("  %s is a %s\r\n", GET_NAME(task_display), p_task_display_);

	g_task_display_cnt = G_TASK_DISPLAY_CNT_INI;

	/* Print out: Task execution counter */
	LOGGER_LOG("   %s = %lu\r\n", GET_NAME(g_task_display_cnt), g_task_display_cnt);

	init_queue_cmd_task_display();

	I2C_LCD_Init(I2C_LCD_1);

	g_task_display_tick_cnt = G_TASK_DISPLAY_TICK_CNT_INI;
}

void task_display_update(void *parameters)
{
	bool b_time_update_required = false;

	/* Update Task display Counter */
	g_task_display_cnt++;

	/* Protect shared resource (g_task_display_tick) */
	__asm("CPSID i");	/* disable interrupts*/
    if (G_TASK_DISPLAY_TICK_CNT_INI < g_task_display_tick_cnt)
    {
    	g_task_display_tick_cnt--;
    	b_time_update_required = true;
    }
    __asm("CPSIE i");	/* enable interrupts*/

    while (b_time_update_required)
    {
//		/* Protect shared resource (g_task_display_tick) */
//		__asm("CPSID i");	/* disable interrupts*/
//		if (G_TASK_DISPLAY_TICK_CNT_INI < g_task_display_tick_cnt)
//		{
//			g_task_display_tick_cnt--;
//			b_time_update_required = true;
//		}
//		else
//		{
//			b_time_update_required = false;
//		}
//		__asm("CPSIE i");	/* enable interrupts*/

    	b_time_update_required = false;

		if (true == any_submcd_task_display())
		{
			task_disp_subcmd_dta_t subcmd_dta = get_subcmd_task_display();

			switch (subcmd_dta.subcmd)
			{
			case SUBCMD_DISP_MOVE_TO:
				I2C_LCD_SetCursor(I2C_LCD_1, 0, subcmd_dta.line);
				break;

			case SUBCMD_DISP_WRITE_CHAR:
				I2C_LCD_WriteChar(I2C_LCD_1, subcmd_dta.chr);
				break;

			default:
				break;
			}
		}
    }
}

/********************** end of file ******************************************/
