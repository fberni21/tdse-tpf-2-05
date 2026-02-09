/*
 * @file   : utils.h
 * @date   : Jan 19, 2026
 * @author : Franco Berni <fberni@fi.uba.ar>
 * @version	v1.0.0
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

#include <stdbool.h>

/********************** macros ***********************************************/

/********************** typedef **********************************************/

/********************** external data declaration ****************************/

/********************** external functions declaration ***********************/
bool is_in_range(uint32_t value, uint32_t min, uint32_t max);

uint32_t temp_raw_to_celsius(uint32_t temp_raw);
uint32_t press_raw_to_kPa(uint32_t press_raw);

void build_status_bar(char out_str[17], uint32_t temp, uint32_t press);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* INC_UTILS_H_ */

/********************** end of file ******************************************/
