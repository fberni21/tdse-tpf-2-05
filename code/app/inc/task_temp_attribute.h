/*
 * @file   : task_temp_attribute.h
 * @date   : Feb 02, 2026
 * @author : Franco Berni <fberni@fi.uba.ar>
 * @version	v1.0.0
 */

#ifndef INC_TASK_TEMP_ATTRIBUTE_H_
#define INC_TASK_TEMP_ATTRIBUTE_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

/********************** macros ***********************************************/

/********************** typedef **********************************************/

/* Events to excite Task Temp */
typedef enum task_temp_ev {EV_TEMP_ENABLE_OFF,
						   EV_TEMP_ENABLE_ON,} task_temp_ev_t;

/* State of Task Temp */
typedef enum task_temp_st {ST_TEMP_OFF,
	   	   	   	   	   	   ST_TEMP_IDLE,
						   ST_TEMP_HEATING,
						   ST_TEMP_COOLING,} task_temp_st_t;

typedef struct
{
	task_temp_st_t	state;
	task_temp_ev_t	event;
	bool			flag;
} task_temp_dta_t;

/********************** external data declaration ****************************/
extern task_temp_dta_t task_temp_dta;

/********************** external functions declaration ***********************/

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* INC_TASK_TEMP_ATTRIBUTE_H_ */

/********************** end of file ******************************************/
