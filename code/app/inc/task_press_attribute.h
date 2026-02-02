/*
 * @file   : task_press_attribute.h
 * @date   : Feb 02, 2026
 * @author : Franco Berni <fberni@fi.uba.ar>
 * @version	v1.0.0
 */

#ifndef INC_TASK_PRESS_ATTRIBUTE_H_
#define INC_TASK_PRESS_ATTRIBUTE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

/********************** macros ***********************************************/

/********************** typedef **********************************************/

/* Events to excite Task Temp */
typedef enum task_press_ev {EV_PRESS_ENABLE_OFF,
						    EV_PRESS_ENABLE_ON,} task_press_ev_t;

/* State of Task Temp */
typedef enum task_press_st {ST_PRESS_OFF,
	   	   	   	   	   	    ST_PRESS_IDLE,
						    ST_PRESS_VACUUM,
						    ST_PRESS_RELEASE,} task_press_st_t;

typedef struct
{
	task_press_st_t	state;
	task_press_ev_t	event;
	bool			flag;
} task_press_dta_t;

/********************** external data declaration ****************************/
extern task_press_dta_t task_press_dta;

/********************** external functions declaration ***********************/

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* INC_TASK_PRESS_ATTRIBUTE_H_ */

/********************** end of file ******************************************/
