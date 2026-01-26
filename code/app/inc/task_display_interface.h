/*
 * @file   : task_display_interface.h
 * @date   : Jan 26, 2026
 * @author : Franco Berni <fberni@fi.uba.ar>
 * @version	v1.0.0
 */

#ifndef INC_TASK_DISPLAY_INTERFACE_H_
#define INC_TASK_DISPLAY_INTERFACE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

#include <stdbool.h>

/********************** macros ***********************************************/

/********************** typedef **********************************************/

typedef enum {
	CMD_DISP_TO_LINE_0,
	CMD_DISP_TO_LINE_1,
	CMD_DISP_WRITE_STR
} task_disp_cmd_t;

typedef enum
{
	SUBCMD_DISP_MOVE_TO,
	SUBCMD_DISP_WRITE_CHAR,
} task_disp_subcmd_t;

typedef struct
{
	task_disp_subcmd_t subcmd;
	union {
		char 	chr;
		uint8_t	line;
	};
} task_disp_subcmd_dta_t;

/********************** external data declaration ****************************/

/********************** external functions declaration ***********************/

void init_queue_cmd_task_display(void);

void put_cmd_task_display(task_disp_cmd_t cmd, const char *text);

task_disp_subcmd_dta_t get_subcmd_task_display(void);

bool any_submcd_task_display(void);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* INC_TASK_DISPLAY_INTERFACE_H_ */

/********************** end of file ******************************************/
