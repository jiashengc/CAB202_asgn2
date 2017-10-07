#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <graphics.h>
#include <lcd.h>
#include <macros.h>
#include "lcd_model.h"
#include <avr/interrupt.h>

void setup() {

	// Initialise essentials
	set_clock_speed(CPU_8MHz);
	lcd_init(LCD_DEFAULT_CONTRAST);
	lcd_clear();

	// Initialise timer 3
	TCCR1A = 0;
	TCCR1B = 5;
	TIMSK1 = 1;

	sei();

	// Enable joysticks
	CLEAR_BIT(DDRB, 1); //LEFT
    CLEAR_BIT(DDRB, 7); //DOWN
    CLEAR_BIT(DDRD, 0); //RIGHT
	CLEAR_BIT(DDRD, 1); //UP
	
	// Initialise buttons
	CLEAR_BIT(DDRF, 5); // SW2 LEFT BUTTON
    CLEAR_BIT(DDRF, 6); // SW1

	draw_string(14, 23, "n9901990", FG_COLOUR);

	show_screen();
}

/**
 * ALL HELPER FUNCTIONS
 */

volatile uint32_t overflow_counter = 3;

ISR(TIMER1_OVF_vect) {
	overflow_counter++;
}
 
double elapsed_time(void) {
	return (overflow_counter * 65536.0 + TCNT1) * 1024 / 8000000;
}

char buffer[20];

void draw_double(uint8_t x, uint8_t y, double value, colour_t colour) {
	snprintf(buffer, sizeof(buffer), "%f", value);
	draw_string(x, y, buffer, colour);
}

void draw_int(uint8_t x, uint8_t y, int value, colour_t colour) {
	snprintf(buffer, sizeof(buffer), "%d", value);
	draw_string(x, y, buffer, colour);
}

/**
 * DRIVERS 
 */

void process() {

	if (BIT_IS_SET(PINF, 6)) {
		lcd_clear();

		double time = elapsed_time();
		draw_string(0, 0, "Time = ", FG_COLOUR);
		draw_string(10, 10, "           ", FG_COLOUR);
		draw_double(10, 10, time, FG_COLOUR);
		show_screen();
	}
	
}

int main(void) {
	setup();

	for ( ;; ) {
		process();
		_delay_ms(100);
	}

	return 0;
}
