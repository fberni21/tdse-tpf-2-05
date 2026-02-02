/*
 * File: I2C_LCD.c
 * Driver Name: [[ I2C_LCD Display ]]
 * SW Layer:   ECUAL
 * Created on: Jan 28, 2024
 * Author:     Khaled Magdy
 * -------------------------------------------
 * For More Information, Tutorials, etc.
 * Visit Website: www.DeepBlueMbedded.com
 *
 */

#include "lcd/I2C_LCD.h"
#include "lcd/I2C_LCD_cfg.h"
#include "lcd/Util.h"

/*-----------------------[INTERNAL DEFINITIONS]-----------------------*/
// CMD
#define LCD_CLEARDISPLAY        0x01
#define LCD_RETURNHOME          0x02
#define LCD_ENTRYMODESET        0x04
#define LCD_DISPLAYCONTROL      0x08
#define LCD_CURSORSHIFT         0x10
#define LCD_FUNCTIONSET         0x20
#define LCD_SETCGRAMADDR        0x40
#define LCD_SETDDRAMADDR        0x80
// DISPLAY ENTRY
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00
// DISPLAY CONTROL
#define LCD_DISPLAYON           0x04
#define LCD_DISPLAYOFF          0x00
#define LCD_CURSORON            0x02
#define LCD_CURSOROFF           0x00
#define LCD_BLINKON             0x01
#define LCD_BLINKOFF            0x00
// CURSOR MOTION
#define LCD_DISPLAYMOVE         0x08
#define LCD_CURSORMOVE          0x00
#define LCD_MOVERIGHT           0x04
#define LCD_MOVELEFT            0x00
// FUNCTION SET
#define LCD_8BITMODE            0x10
#define LCD_4BITMODE            0x00
#define LCD_2LINE               0x08
#define LCD_1LINE               0x00
#define LCD_5x10DOTS            0x04
#define LCD_5x8DOTS             0x00
// BACKLIGHT CONTROL
#define LCD_BACKLIGHT           0x08
#define LCD_NOBACKLIGHT         0x00
#define EN                      0b00000100  // Enable bit
#define RW                      0b00000010  // Read/Write bit
#define RS                      0b00000001  // Register select bit

/*-----------------------[INTERNAL VARIABLES]-----------------------*/

typedef struct I2C_LCD_InfoParam_s
{
    uint8_t DisplayCtrl;
    uint8_t BacklightVal;
}I2C_LCD_InfoParam_t;

static I2C_LCD_InfoParam_t I2C_LCD_InfoParam_g[I2C_LCD_MAX];

static I2C_LCD_Queue_t lcd_queue_list[I2C_LCD_MAX];

/*---------------------[STATIC INTERNAL FUNCTIONS]-----------------------*/

static uint8_t i2c_tx_buffer[4];

static void I2C_LCD_Process_Next(uint8_t instance) {
	if (lcd_queue_list[instance].head == lcd_queue_list[instance].tail) {
		lcd_queue_list[instance].is_busy = 0;
		return;
	}

	lcd_queue_list[instance].is_busy = 1;

	uint16_t item = lcd_queue_list[instance].buffer[lcd_queue_list[instance].tail];
	lcd_queue_list[instance].tail = (lcd_queue_list[instance].tail + 1) % LCD_TX_BUFFER_SIZE;

	uint8_t value = (uint8_t)(item & 0x00FF);
	uint8_t rs_bit = (uint8_t)((item >> 8) & RS);

	uint8_t high_nibble = value & 0xF0;
	uint8_t low_nibble = (value << 4) & 0xF0;
	uint8_t bl = I2C_LCD_InfoParam_g[instance].BacklightVal;

	// Estructura: [Nibble | Backlight | RS | EN/RW]
	i2c_tx_buffer[0] = high_nibble | bl | rs_bit | EN; // Enable High
	i2c_tx_buffer[1] = high_nibble | bl | rs_bit;      // Enable Low (Write)

	i2c_tx_buffer[2] = low_nibble  | bl | rs_bit | EN; // Enable High
	i2c_tx_buffer[3] = low_nibble  | bl | rs_bit;      // Enable Low (Write)

	HAL_I2C_Master_Transmit_IT(
		I2C_LCD_CfgParam[instance].I2C_Handle,
		I2C_LCD_CfgParam[instance].I2C_LCD_Address << 1,
		i2c_tx_buffer,
		4
	);
}

static void I2C_LCD_Push(uint8_t instance, uint8_t value, uint16_t type) {
    uint16_t next_head = (lcd_queue_list[instance].head + 1) % LCD_TX_BUFFER_SIZE;

    // TODO: quizás habría que fallar de alguna forma si la cola está llena?
    if (next_head != lcd_queue_list[instance].tail) {
    	lcd_queue_list[instance].buffer[lcd_queue_list[instance].head] = (uint16_t)value | (type << 8);
    	lcd_queue_list[instance].head = next_head;
    }

    if (!lcd_queue_list[instance].is_busy) {
        I2C_LCD_Process_Next(instance);
    }
}

//  Solo para inicialización porque son bloqueantes

static void I2C_LCD_ExpanderWrite(uint8_t I2C_LCD_InstanceIndex, uint8_t DATA)
{
    uint8_t TxData = (DATA) | I2C_LCD_InfoParam_g[I2C_LCD_InstanceIndex].BacklightVal;
    HAL_I2C_Master_Transmit(I2C_LCD_CfgParam[I2C_LCD_InstanceIndex].I2C_Handle, (I2C_LCD_CfgParam[I2C_LCD_InstanceIndex].I2C_LCD_Address<<1), &TxData, sizeof(TxData), 100);

}

static void I2C_LCD_EnPulse(uint8_t I2C_LCD_InstanceIndex, uint8_t DATA)
{
	I2C_LCD_ExpanderWrite(I2C_LCD_InstanceIndex, (DATA | EN)); // En high
	DELAY_US(2); // enable pulse must be >450ns

    I2C_LCD_ExpanderWrite(I2C_LCD_InstanceIndex, (DATA & ~EN)); // En low
    DELAY_US(50); // commands need > 37us to settle
}

static void I2C_LCD_Write4Bits(uint8_t I2C_LCD_InstanceIndex, uint8_t Val)
{
	I2C_LCD_ExpanderWrite(I2C_LCD_InstanceIndex, Val);
	I2C_LCD_EnPulse(I2C_LCD_InstanceIndex, Val);
}

static void I2C_LCD_Send(uint8_t I2C_LCD_InstanceIndex, uint8_t Val, uint8_t Mode)
{
    uint8_t HighNib = Val & 0xF0;
    uint8_t LowNib = (Val << 4) & 0xF0;
    I2C_LCD_Write4Bits(I2C_LCD_InstanceIndex, (HighNib) | Mode);
    I2C_LCD_Write4Bits(I2C_LCD_InstanceIndex, (LowNib) | Mode);
}

static void I2C_LCD_Cmd(uint8_t I2C_LCD_InstanceIndex, uint8_t CMD)
{
	I2C_LCD_Send(I2C_LCD_InstanceIndex, CMD, 0);
}

/*-----------------------------------------------------------------------*/

//=========================================================================================================================

/*-----------------------[USER EXTERNAL FUNCTIONS]-----------------------*/

void I2C_LCD_Init(uint8_t I2C_LCD_InstanceIndex)
{
	// According To Datasheet, We Must Wait At Least 40ms After Power Up Before Interacting With The LCD Module
	while(HAL_GetTick() < 50);
    I2C_LCD_Cmd(I2C_LCD_InstanceIndex, 0x30);
    DELAY_MS(5);  // Delay > 4.1ms
    I2C_LCD_Cmd(I2C_LCD_InstanceIndex, 0x30);
    DELAY_MS(5);  // Delay > 4.1ms
    I2C_LCD_Cmd(I2C_LCD_InstanceIndex, 0x30);
    DELAY_US(150);  // Delay > 100μs
    I2C_LCD_Cmd(I2C_LCD_InstanceIndex, 0x02);
    // Configure the LCD
    I2C_LCD_Cmd(I2C_LCD_InstanceIndex, LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
    I2C_LCD_Cmd(I2C_LCD_InstanceIndex, LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
    I2C_LCD_Cmd(I2C_LCD_InstanceIndex, LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
    I2C_LCD_InfoParam_g[I2C_LCD_InstanceIndex].DisplayCtrl = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    I2C_LCD_InfoParam_g[I2C_LCD_InstanceIndex].BacklightVal = LCD_BACKLIGHT;
    // Clear the LCD
    I2C_LCD_Clear(I2C_LCD_InstanceIndex);
}


void I2C_LCD_Clear(uint8_t I2C_LCD_InstanceIndex)
{
    I2C_LCD_Cmd(I2C_LCD_InstanceIndex, LCD_CLEARDISPLAY);
    DELAY_MS(2);
}

void I2C_LCD_SetCursor(uint8_t instance, uint8_t Col, uint8_t Row) {
    int Row_Offsets[] = {0x00, 0x40, 0x14, 0x54};

    if (Row >= I2C_LCD_CfgParam[instance].I2C_LCD_nRow) {
        Row = I2C_LCD_CfgParam[instance].I2C_LCD_nRow - 1;
    }

    uint8_t command = LCD_SETDDRAMADDR | (Col + Row_Offsets[Row]);

    I2C_LCD_Push(instance, command, 0);
}

void I2C_LCD_WriteChar(uint8_t instance, char Ch) {
    I2C_LCD_Push(instance, (uint8_t)Ch, RS);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
	// FIXME: funciona solo para un LCD, igual solo usamos uno así que creo que podría quedar así.
    if (hi2c == I2C_LCD_CfgParam[I2C_LCD_1].I2C_Handle) {
        I2C_LCD_Process_Next(I2C_LCD_1);
    }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    while (1)
    {
    }
}
