/*
 * @file   : eeprom.h
 * @date   : Jan 19, 2026
 * @author : Franco Berni <fberni@fi.uba.ar>
 * @version	v1.0.0
 */

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

/********************** macros ***********************************************/

#define EEPROM_MAX_ADDRESS 63999

/********************** typedef **********************************************/

/********************** external data declaration ****************************/

/********************** external functions declaration ***********************/
void eeprom_write(uint8_t offset, void *data, size_t size);
void eeprom_read(uint8_t offset, void *data, size_t size);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* INC_EEPROM_H_ */

/********************** end of file ******************************************/
