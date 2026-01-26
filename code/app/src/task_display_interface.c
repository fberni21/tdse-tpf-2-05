/*
 * @file   : task_display_interface.c
 * @date   : Jan 26, 2026
 * @author : Franco Berni <fberni@fi.uba.ar>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/
#include "main.h"

#include "task_display_interface.h"

/********************** macros and definitions *******************************/
#define SUBCMD_UNDEFINED	(255)
#define MAX_SUBCMDS		(64)

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

void put_subcmd_task_display(task_disp_subcmd_dta_t cmd);

/********************** internal data definition *****************************/

struct
{
	uint32_t	head;
	uint32_t	tail;
	uint32_t	count;
	task_disp_subcmd_dta_t	queue[MAX_SUBCMDS];
} queue_task_disp;

/********************** external data declaration ****************************/

/********************** external functions definition ************************/
void init_queue_cmd_task_display(void)
{
	uint32_t i;

	queue_task_disp.head = 0;
	queue_task_disp.tail = 0;
	queue_task_disp.count = 0;

	for (i = 0; i < MAX_SUBCMDS; i++)
	{
		queue_task_disp.queue[i].subcmd = SUBCMD_UNDEFINED;
		queue_task_disp.queue[i].line = 0;
	}
}

void put_cmd_task_display(task_disp_cmd_t cmd, const char *text)
{
	task_disp_subcmd_dta_t subcmd_dta;

	switch (cmd)
	{
	case CMD_DISP_TO_LINE_0:
		subcmd_dta.subcmd = SUBCMD_DISP_MOVE_TO;
		subcmd_dta.line = 0;
		put_subcmd_task_display(subcmd_dta);
		break;

	case CMD_DISP_TO_LINE_1:
		subcmd_dta.subcmd = SUBCMD_DISP_MOVE_TO;
		subcmd_dta.line = 1;
		put_subcmd_task_display(subcmd_dta);
		break;

	case CMD_DISP_WRITE_STR:
		subcmd_dta.subcmd = SUBCMD_DISP_WRITE_CHAR;
		while (*text)
		{
			subcmd_dta.chr = *text++;
			put_subcmd_task_display(subcmd_dta);
		}
	}
}

void put_subcmd_task_display(task_disp_subcmd_dta_t subcmd)
{
	queue_task_disp.count++;
	queue_task_disp.queue[queue_task_disp.head++] = subcmd;

	if (MAX_SUBCMDS == queue_task_disp.head)
		queue_task_disp.head = 0;
}

task_disp_subcmd_dta_t get_subcmd_task_display(void)

{
	task_disp_subcmd_dta_t subcmd_dta;

	queue_task_disp.count--;
	subcmd_dta = queue_task_disp.queue[queue_task_disp.tail];
	queue_task_disp.queue[queue_task_disp.tail].subcmd = SUBCMD_UNDEFINED;
	queue_task_disp.queue[queue_task_disp.tail].line = 0;
	queue_task_disp.tail++;

	if (MAX_SUBCMDS == queue_task_disp.tail)
		queue_task_disp.tail = 0;

	return subcmd_dta;
}

bool any_submcd_task_display(void)
{
  return (queue_task_disp.head != queue_task_disp.tail);
}

/********************** end of file ******************************************/
