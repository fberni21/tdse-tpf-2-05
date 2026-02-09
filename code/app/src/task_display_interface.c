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
#define SUBCMD_UNDEFINED	'\xFF'

#define MAX_SUBCMDS			(64)

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

void put_subcmd_task_display(char subcmd);

/********************** internal data definition *****************************/

struct
{
	uint32_t	head;
	uint32_t	tail;
	uint32_t	count;
	char		queue[MAX_SUBCMDS];
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
		queue_task_disp.queue[i] = SUBCMD_UNDEFINED;
}

void put_cmd_task_display(task_disp_cmd_t cmd, const char *text)
{
	switch (cmd)
	{
	case CMD_DISP_TO_LINE_0:
		put_subcmd_task_display(SUBCMD_LINE_0);
		break;

	case CMD_DISP_TO_LINE_1:
		put_subcmd_task_display(SUBCMD_LINE_1);
		break;

	case CMD_DISP_WRITE_STR:
		while (*text)
			put_subcmd_task_display(*text++);
	}
}

void put_subcmd_task_display(char subcmd)
{
	queue_task_disp.count++;
	queue_task_disp.queue[queue_task_disp.head++] = subcmd;

	if (MAX_SUBCMDS == queue_task_disp.head)
		queue_task_disp.head = 0;
}

char get_subcmd_task_display(void)

{
	char subcmd_dta;

	queue_task_disp.count--;
	subcmd_dta = queue_task_disp.queue[queue_task_disp.tail];
	queue_task_disp.queue[queue_task_disp.tail] = SUBCMD_UNDEFINED;
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
