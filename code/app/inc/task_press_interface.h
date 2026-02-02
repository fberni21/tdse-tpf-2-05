/*
 * @file   : task_press_interface.h
 * @date   : Feb 02, 2026
 * @author : Franco Berni <fberni@fi.uba.ar>
 * @version	v1.0.0
 */

#ifndef INC_TASK_PRESS_INTERFACE_H_
#define INC_TASK_PRESS_INTERFACE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

#include "task_press_attribute.h"

/********************** macros ***********************************************/

/********************** typedef **********************************************/

/********************** external data declaration ****************************/

/********************** external functions declaration ***********************/
extern void init_queue_event_task_press(void);
extern void put_event_task_press(task_press_ev_t event);
extern task_press_ev_t get_event_task_press(void);
extern bool any_event_task_press(void);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* INC_TASK_PRESS_INTERFACE_H_ */

/********************** end of file ******************************************/
