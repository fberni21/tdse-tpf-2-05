/*
 * @file   : task_temp_interface.c
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

/********************** macros and definitions *******************************/
#define EVENT_UNDEFINED	(255)
#define MAX_EVENTS		(16)

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
struct
{
	uint32_t	head;
	uint32_t	tail;
	uint32_t	count;
	task_temp_ev_t	queue[MAX_EVENTS];
} queue_task_temp;

/********************** external data declaration ****************************/

/********************** external functions definition ************************/
void init_queue_event_task_temp(void)
{
	uint32_t i;

	queue_task_temp.head = 0;
	queue_task_temp.tail = 0;
	queue_task_temp.count = 0;

	for (i = 0; i < MAX_EVENTS; i++)
		queue_task_temp.queue[i] = EVENT_UNDEFINED;
}

void put_event_task_temp(task_temp_ev_t event)
{
	queue_task_temp.count++;
	queue_task_temp.queue[queue_task_temp.head++] = event;

	if (MAX_EVENTS == queue_task_temp.head)
		queue_task_temp.head = 0;
}

task_temp_ev_t get_event_task_temp(void)

{
	task_temp_ev_t event;

	queue_task_temp.count--;
	event = queue_task_temp.queue[queue_task_temp.tail];
	queue_task_temp.queue[queue_task_temp.tail++] = EVENT_UNDEFINED;

	if (MAX_EVENTS == queue_task_temp.tail)
		queue_task_temp.tail = 0;

	return event;
}

bool any_event_task_temp(void)
{
  return (queue_task_temp.head != queue_task_temp.tail);
}

/********************** end of file ******************************************/
