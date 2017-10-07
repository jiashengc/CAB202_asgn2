#include <avr/io.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <graphics.h>
#include <lcd.h>
#include <macros.h>
#include "lcd_model.h"

void new_lcd_init(uint8_t contrast) {
	// Set up the pins connected to the LCD as outputs
	SET_OUTPUT(DDRD, SCEPIN); // Chip select -- when low, tells LCD we're sending data
	SET_OUTPUT(DDRB, RSTPIN); // Chip Reset
	SET_OUTPUT(DDRB, DCPIN);  // Data / Command selector
	SET_OUTPUT(DDRB, DINPIN); // Data input to LCD
	SET_OUTPUT(DDRF, SCKPIN); // Clock input to LCD

	CLEAR_BIT(PORTB, RSTPIN); // Reset LCD
	SET_BIT(PORTD, SCEPIN);   // Tell LCD we're not sending data.
	SET_BIT(PORTB, RSTPIN);   // Stop resetting LCD

	LCD_CMD(lcd_set_function, lcd_instr_extended);
	LCD_CMD(lcd_set_contrast, contrast);
	LCD_CMD(lcd_set_temp_coeff, 0);
	LCD_CMD(lcd_set_bias, 3);
	LCD_CMD(lcd_set_function, lcd_instr_basic);
	LCD_CMD(lcd_set_display_mode, lcd_display_normal);
	LCD_CMD(lcd_set_x_addr, 0);
	LCD_CMD(lcd_set_y_addr, 0);
}

void setup() {

}

void process() {

}

int main(void) {
	setup();

	for ( ;; ) {
		process();
		_delay_ms(100);
	}

	return 0;
}
