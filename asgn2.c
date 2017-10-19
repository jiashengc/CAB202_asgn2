#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <math.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <graphics.h>
#include <lcd.h>
#include <macros.h>
#include "sprite.h"
#include "lcd_model.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "usb_serial.h"
#include "string.h"

#define walls_n 0
#define treasure_n 5
#define mob_n 5

uint8_t level = 0;
uint8_t lives = 3;
uint8_t hero_speed = 2;
uint8_t minutes = 0;
uint8_t tenth_seconds = 0;
uint8_t seconds = 0;
uint16_t score = 0;

uint8_t o_minutes = 0;
uint8_t o_tenth_seconds = 0;
uint8_t o_seconds = 0;

uint8_t has_shield = 0;
uint8_t has_key = 0;

uint8_t hero_x = 0;
uint8_t hero_y = 0;

uint8_t mob_init[mob_n] = {0};
char last_direction;
char *message = "";

// Initialise sprites
Sprite hero;
Sprite tower;
Sprite door;
Sprite key;
Sprite mob[mob_n];
Sprite treasure[treasure_n];
Sprite shield;
Sprite wall_left;
Sprite wall_right;
Sprite wall_top;
Sprite wall_bot;

Sprite wall[walls_n];

// Sprite vectors
uint8_t hero_bitmap[] = {
	0b00101000,
	0b00111000,
	0b00101000,
	0b00111000,
	0b00000000,
	0b11111110,
	0b10101010,
	0b00111000,
	0b00101000,
	0b00101000
};

uint8_t tower_bitmap[] = {
	0b01111111, 0b00000000, 0b11111111, 0b00000000, 0b11111111, 0b00000000, 0b11111111, 0b00000001, 0b11111110,
	0b01000001, 0b00000000, 0b10000001, 0b00000000, 0b10000001, 0b00000000, 0b10000001, 0b00000000, 0b10000010,
	0b01000001, 0b11111111, 0b10000001, 0b11111111, 0b10000001, 0b11111111, 0b10000001, 0b11111111, 0b10000010,
	0b01000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b01000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b01000000, 0b00000000, 0b01111100, 0b00000000, 0b01111110, 0b00000000, 0b01111100, 0b00000000, 0b00000010,
	0b01100000, 0b00000000, 0b01000100, 0b00000000, 0b01000010, 0b00000000, 0b01000100, 0b00000000, 0b00000110,
	0b01100000, 0b00000000, 0b01000100, 0b00000000, 0b01000010, 0b00000000, 0b01000100, 0b00000000, 0b00000110,
	0b01000000, 0b00000000, 0b01111100, 0b00000000, 0b01111110, 0b00000000, 0b01111100, 0b00000000, 0b00000010,
	0b01000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b01000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b01000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b01000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
	0b01100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000110,
	0b00111000, 0b00000000, 0b00000000, 0b00000000, 0b00111100, 0b00000000, 0b00000000, 0b00000000, 0b00011100,
	0b00011100, 0b00000000, 0b00000000, 0b00000000, 0b11000011, 0b00000000, 0b00000000, 0b00000000, 0b00111000,
	0b00000111, 0b00000000, 0b00000000, 0b00000001, 0b00000000, 0b10000000, 0b00000000, 0b00000000, 0b11100000,
	0b00000001, 0b10000000, 0b00000000, 0b00000001, 0b00000000, 0b10000000, 0b00000000, 0b00000001, 0b10000000,
	0b00000000, 0b11000000, 0b00000000, 0b00000010, 0b00000000, 0b01000000, 0b00000000, 0b00000011, 0b00000000,
	0b00000000, 0b01110000, 0b00000000, 0b00000100, 0b00000011, 0b00100000, 0b00000000, 0b00001110, 0b00000000,
	0b00000000, 0b00011110, 0b00000000, 0b00000100, 0b00000011, 0b00100000, 0b00000000, 0b01111000, 0b00000000,
	0b00000000, 0b00000001, 0b11100000, 0b00000100, 0b00000000, 0b00100000, 0b00000111, 0b10000000, 0b00000000,
	0b00000000, 0b00000000, 0b00111111, 0b00000100, 0b00000000, 0b00100000, 0b11111100, 0b00000000, 0b00000000,
	0b00000000, 0b00000000, 0b00000111, 0b11111100, 0b00000000, 0b00111111, 0b11100000, 0b00000000, 0b00000000
};

uint8_t door_bitmap[] = {
	0b00000000, 0b00111100, 0b00000000,
	0b00000000, 0b11000011, 0b00000000,
	0b00000001, 0b00000000, 0b10000000,
	0b00000001, 0b00000000, 0b10000000,
	0b00000010, 0b00000000, 0b01000000,
	0b00000100, 0b00000011, 0b00100000,
	0b00000100, 0b00000011, 0b00100000,
	0b00000100, 0b00000000, 0b00100000,
	0b00000100, 0b00000000, 0b00100000,
	0b11111100, 0b00000000, 0b00111111
};

uint8_t mob_bitmap[] = {
	0b01110000,
	0b11111000,
	0b10101000,
	0b11111000,
	0b11111000,
	0b10101000,
};

uint8_t key_bitmap[] = {
	0b11100000,
	0b10111110,
	0b11101010,
};

uint8_t treasure_bitmap[] = {
	0b00100000,
	0b01110000,
	0b11111000,
};

uint8_t shield_bitmap[] = {
	0b11110000,
	0b11110000,
	0b11110000,
	0b01100000,
};

uint8_t wall_top_bitmap[] = {
	0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 
	0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 
	0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 
};

uint8_t wall_left_bitmap[] = {
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,	
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,	
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,	
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
	0b11100000,
};

void sprite_set_speed(sprite_id sprite, float dx, float dy);
void setup_usb_serial(void);
// sprite_id sprite_create(float x, float y, uint8_t width, uint8_t height, uint8_t bitmap[]);

int estimate_ram(void){
	extern int __heap_start, *__brkval;
	int v;
	return (int) (&v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
  }

void setup() {
	uint8_t contrast = 175;

	// Initialise lights
	SET_BIT(DDRC, 7); // LCD LED
	SET_BIT(PORTC, 7);

	// Enable centre LED for output
	SET_BIT(DDRD, 6);

	// Initialise essentials
	set_clock_speed(CPU_8MHz);
	lcd_init(LCD_DEFAULT_CONTRAST);
	lcd_clear();

	// Initialise timer 3
	// TCCR1A = 0;
	// TCCR1B = 5;
	// TIMSK1 = 1;

	// Set Timer 0 to overflow approx 122 times per second.
	TCCR0B |= 4;
	TIMSK0 = 1;
	
	sei();

	setup_usb_serial();

	// Enable joysticks
	CLEAR_BIT(DDRB, 1); //LEFT
    CLEAR_BIT(DDRB, 7); //DOWN
    CLEAR_BIT(DDRD, 0); //RIGHT
	CLEAR_BIT(DDRD, 1); //UP
	
	// Initialise buttons
	CLEAR_BIT(DDRF, 5); // SW3 RIGHT BUTTON
	CLEAR_BIT(DDRF, 6); // SW2 LEFT BUTTON
	CLEAR_BIT(DDRB, 0); // CENTRE BUTTON

	// Initialise backlight
	SET_BIT(DDRC, 7);

	// Initialise contrast
	lcd_init(contrast);

	// Initialise sprites
	sprite_init(&hero, LCD_X / 2, LCD_Y / 2, 7, 10, hero_bitmap);
	sprite_init(&tower, 6, -10, 72, 26, tower_bitmap);
	sprite_init(&door, 30, 5, 24, 12, door_bitmap);
	sprite_init(&mob[0], 72, 20, 5, 6, mob_bitmap);
	sprite_init(&key, 6, 20, 7, 3, key_bitmap);

	// Initialise walls 
	sprite_init(&wall_left, -40, -40, 3, 100, wall_left_bitmap);
	sprite_init(&wall_top, -40, -40, 160, 3, wall_top_bitmap);
	sprite_init(&wall_right, 120, -40, 3, 100, wall_left_bitmap);
 	sprite_init(&wall_bot, -40, 60, 160, 3, wall_top_bitmap);

	// Initialise sprite movements
	sprite_set_speed(&hero, hero_speed, hero_speed);
	sprite_set_speed(&mob[0], .25, .25);

	show_screen();
}

void usb_serial_send(char * message) {
	// Cast to avoid "error: pointer targets in passing argument 1 
	//	of 'usb_serial_write' differ in signedness"
	usb_serial_write((uint8_t *) message, strlen(message));
}

void setup_usb_serial(void) {
	// Set up LCD and display message
	draw_string(5, 10, "Connect Serial", FG_COLOUR);
	show_screen();

	usb_init();

	//while (!usb_serial_available()) {
		// Block until USB is ready.
	//}

	message = "Welcome";
	usb_serial_send(message);

	clear_screen();
	draw_string(2, 10, "Serial connected", FG_COLOUR);
	show_screen();
}

/**
 * ALL HELPER FUNCTIONS
 */

// volatile uint32_t overflow_counter = 3;

// ISR(TIMER1_OVF_vect) {
// 	overflow_counter++;
// }
 
// double elapsed_time(void) {
// 	return (overflow_counter * 65536.0 + TCNT1) * 1024 / 8000000;
// }

char buffer[20];

void draw_double(uint8_t x, uint8_t y, double value, colour_t colour) {
	snprintf(buffer, sizeof(buffer), "%f", value);
	draw_string(x, y, buffer, colour);
}

void draw_int(uint8_t x, uint8_t y, int value, colour_t colour) {
	snprintf(buffer, sizeof(buffer), "%d", value);
	draw_string(x, y, buffer, colour);
}

// sprite_id sprite_create(float x, float y, uint8_t width, uint8_t height, uint8_t bitmap[]) {
// 	sprite_id sprite = malloc( sizeof( Sprite ) );

// 	if (&sprite != NULL ) {
// 		sprite_init(&sprite, x, y, width, height, bitmap );
// 	}

// 	return sprite;
// }

void sprite_set_speed(sprite_id sprite, float dx, float dy) {
	sprite->dx = dx;
	sprite->dy = dy;
	return;
}

void sprite_move_all(sprite_id sprite, char direction) {
	last_direction = direction;
	switch(direction) {
		case 'L':
			sprite->x += hero_speed;
			break;
		case 'R':
			sprite->x -= hero_speed;
			break;
		case 'U':
			sprite->y += hero_speed;
			break;
		case 'D':
			sprite->y -= hero_speed;
			break;
	}
	return;
}

void move_all(char direction) {
	uint8_t i = 0;
	if (&tower != NULL) {
		sprite_move_all(&tower, direction);
	}
	for (i = 0; i < treasure_n; i++) {
		if (&treasure[i] != NULL) {
			sprite_move_all(&treasure[i], direction);
		} 
	}
	
	if (&shield != NULL) {
		sprite_move_all(&shield, direction);
	}

	for (i = 0; i < mob_n; i++) {
		if (&mob != NULL) {
			sprite_move_all(&mob[i], direction);
		}
	}
	
	sprite_move_all(&key, direction);
	sprite_move_all(&door, direction);
	sprite_move_all(&wall_left, direction);	
	sprite_move_all(&wall_top, direction);
	sprite_move_all(&wall_right, direction);
	sprite_move_all(&wall_bot, direction);
	
	if (level == 1) {
		return;
	}

	for (i = 0; i < walls_n; i++) {
		sprite_move_all(&wall[i], direction);
	}
}

int sprite_back(sprite_id sprite) {
	int x0 = round( sprite->x );
	int y0 = round( sprite->y );
	sprite->x -= sprite->dx;
	sprite->y -= sprite->dy;
	int x1 = round( sprite->x );
	int y1 = round( sprite->y );
	return ( x1 != x0 ) || ( y1 != y0 );
}

void sprite_back_all() {
	char direction = ' ';
	if (last_direction == 'L') {
		direction = 'R';
	}
	else if (last_direction == 'R') {
		direction = 'L';
	}
	else if (last_direction == 'U') {
		direction = 'D';
	} 
	else if (last_direction == 'D') {
		direction = 'U';
	}
	
	move_all(direction);
}

void sprite_move(sprite_id sprite, char direction) {
	switch(direction) {
		case 'L':
			if (sprite->dx > 0) {
				sprite->dx = -sprite->dx;
			}
			sprite->x += sprite->dx;
			break;
		case 'R':
			if (sprite->dx < 0) {
				sprite->dx = -sprite->dx;
			}
			sprite->x += sprite->dx;
			break;
		case 'U':
			if (sprite->dy > 0) {
				sprite->dy = -sprite->dy;
			} 
			sprite->y += sprite->dy;
			break;
		case 'D':
			if (sprite->dy < 0) {
				sprite->dy = -sprite->dy;
			}
			sprite->y += sprite->dy;
			break;
	}
	return;
}

void sprite_visible( sprite_id sprite, uint8_t visible) {
	sprite->is_visible = visible;
	return;
}

int sprite_is_visible(sprite_id sprite) {
	return sprite->is_visible;
}

int sprite_move_to( sprite_id sprite, double x, double y ) {
	int x0 = round( sprite->x );
	int y0 = round( sprite->y );
	sprite->x = x;
	sprite->y = y;
	int x1 = round( sprite->x );
	int y1 = round( sprite->y );
	return ( x1 != x0 ) || ( y1 != y0 );
}

float sprite_x( sprite_id sprite ) {
	return sprite->x;
}

float sprite_y( sprite_id sprite ) {
	return sprite->y;
}

float sprite_dx (sprite_id sprite) {
	return sprite->dx;
}

float sprite_dy(sprite_id sprite) {
	return sprite->dy;
}

void sprite_set_xy(sprite_id sprite, int x, int y) {
	sprite->x = x;
	sprite->y = y;
}

void mob_move(sprite_id hero, sprite_id mob) {
	if (!sprite_is_visible(hero)) {
		return;
	}

	float hx = sprite_x(hero);
	float hy = sprite_y(hero);
	float mx = sprite_x(mob);
	float my = sprite_y(mob);

	// If the mob is to the right of the hero
	if (hx < mx) {
		sprite_move(mob, 'L');
	} else {
		sprite_move(mob, 'R');
	}

	// If the mob is below the hero
	if (hy < my) {
		sprite_move(mob, 'U');
	} else {
		sprite_move(mob, 'D');
	}
}

void sprite_follow(sprite_id sprite_1, sprite_id sprite_2) {
	sprite_1->x = sprite_2->x - (sprite_2->width / 2) - 1;
	sprite_1->y = sprite_2->y + (sprite_2->height / 2);
}

void sprite_follow_2(sprite_id sprite_1, sprite_id sprite_2) {
	sprite_1->x = sprite_2->x - (sprite_2->width / 2) + 6;
	sprite_1->y = sprite_2->y + (sprite_2->height / 2) - 1;
}

uint8_t sprite_width( sprite_id sprite ) {
	return sprite->width;
}

uint8_t sprite_height( sprite_id sprite ) {
	return sprite->height;
}

void sprite_destroy( sprite_id sprite ) {
	sprite->is_visible = 0;
	sprite->x = rand() % 500 + 200;
	sprite->y = rand() % 500 + 200;
	sprite = NULL;
	free(sprite);
}

int sprite_exists(sprite_id sprite) {
	return sprite != NULL || sprite->is_visible != 0;
}

int process_collision(sprite_id obj_1, sprite_id obj_2) {

	// Get platform and bird screen locations.
	int hx = round(sprite_x(obj_1)), hy = round(sprite_y(obj_1));
	int ox = round(sprite_x(obj_2)), oy = round(sprite_y(obj_2));

	// Check for collision
	int collided = 1;

	if ( hx >= ox + sprite_width(obj_2)) collided = 0;
	if ( hy >= oy + sprite_height(obj_2)) collided = 0;
	if ( ox >= hx + sprite_width(obj_1)) collided = 0;
	if ( oy >= hy + sprite_height(obj_1)) collided = 0;

	if (!collided || obj_2 == &key || obj_2 == &shield) {
		return collided;
	}

	if (obj_1 == &hero) {
		sprite_back_all();
	}

	uint8_t i = 0;
	for (i = 0; i < mob_n; i++) {
		if (obj_1 == &mob[i]) {
			sprite_back(obj_1);
		}
	}

  return collided;
}

/**
 * DRIVERS 
 */

void game_over_screen(void);
void loading_screen(void);

uint8_t door_x;
uint8_t door_y;
uint8_t mob_x[mob_n];
uint8_t mob_y[mob_n];
uint8_t key_x;
uint8_t key_y;
uint8_t treasure_x[treasure_n];
uint8_t treasure_y[treasure_n];
uint8_t shield_x;
uint8_t shield_y;
uint8_t wall_x[walls_n];
uint8_t wall_y[walls_n];

void restart_level() {
	has_shield = 0;
	has_key = 0;

	uint8_t i = 0;
	lives -= 1;

	sprite_destroy(&wall_left);
	sprite_destroy(&wall_top);
	sprite_destroy(&wall_right);
	sprite_destroy(&wall_bot);

	// GUARD: Check if the lives is 0
	if (lives <= 0) {
		game_over_screen();
	}

	if (level == 1) {
		for (i = 0; i < mob_n; i++) {
			if (sprite_exists(&mob[i])) {
				sprite_destroy(&mob[i]);
			}
		}

		sprite_destroy(&hero);
		sprite_destroy(&tower);
		sprite_destroy(&door);
		sprite_destroy(&key);

		// Initialise sprites
		sprite_init(&hero, LCD_X / 2, LCD_Y / 2, 7, 10, hero_bitmap);
		sprite_init(&tower, 6, -10, 72, 26, tower_bitmap);
		sprite_init(&door, 30, 5, 24, 12, door_bitmap);
		sprite_init(&mob[0], 72, 20, 5, 6, mob_bitmap);
		sprite_init(&key, 6, 20, 7, 3, key_bitmap);

		// Initialise walls 
		sprite_init(&wall_left, -40, -40, 3, 100, wall_left_bitmap);
		sprite_init(&wall_top, -40, -40, 160, 3, wall_top_bitmap);
		sprite_init(&wall_right, 120, -40, 3, 100, wall_left_bitmap);
		sprite_init(&wall_bot, -40, 60, 160, 3, wall_top_bitmap);

		// Initialise sprite movements
		sprite_set_speed(&hero, hero_speed, hero_speed);
		sprite_set_speed(&mob[0], .25, .25);
	}

	if (level > 1) {
		for (i = 0; i < mob_n; i++) {
			if (sprite_exists(&mob[i])) {
				sprite_destroy(&mob[i]);
			}
		}

		sprite_destroy(&hero);
		sprite_destroy(&door);
		sprite_destroy(&key);

		sprite_init(
			&door,
			door_x,
			door_y, 
			24, 12, door_bitmap
		);
		
		sprite_init(
			&key,
			key_x,
			key_y,
			7, 3, key_bitmap
		);
	
		for (i = 0; i < treasure_n; i++) {
			if (sprite_exists(&treasure[i])) {
				sprite_destroy(&treasure[i]);
				sprite_init(
					&treasure[i],
					treasure_x[i],
					treasure_y[i],
					5, 3, treasure_bitmap
				);
			}
		}
		
		sprite_init(
			&hero,
			LCD_X / 2, 
			LCD_Y / 2, 
			7, 10, hero_bitmap
		);

		sprite_set_speed(&hero, hero_speed, hero_speed);

		uint8_t n = 0;

		for (n = 0; n < mob_n; n++) {
			if (sprite_exists(&mob[n])) {
				sprite_destroy(&mob[n]);
				sprite_init(
					&mob[n],
					mob_x[n],
					mob_y[n],
					5, 6, mob_bitmap
				);
				sprite_set_speed(&mob[n], .25, .25);
			} else {
				sprite_visible(&mob[n], 0);
			}
		}

		// Initialise walls 
		sprite_init(&wall_left, -40, -40, 3, 100, wall_left_bitmap);
		sprite_init(&wall_top, -40, -40, 160, 3, wall_top_bitmap);
		sprite_init(&wall_right, 120, -40, 3, 100, wall_left_bitmap);
		sprite_init(&wall_bot, -40, 60, 160, 3, wall_top_bitmap);
		
	}

}

void next_level() {
	has_shield = 0;
	has_key = 0;
	uint8_t i;

	clear_screen();
	loading_screen();

	score += 100;
	level += 1;
	if (sprite_exists(&tower)) {
		sprite_destroy(&tower);
	}
	
	// Create hero
	sprite_destroy(&hero);
	sprite_init(
		&hero,
		LCD_X / 2, 
		LCD_Y / 2, 
		7, 10, hero_bitmap
	);

	do {
		sprite_destroy(&door);
		door_x = rand() % LCD_X;
		door_y = rand() % LCD_Y;
		sprite_init(
			&door,
			door_x,
			door_y, 
			24, 12, door_bitmap
		);
	} while (process_collision(&door, &hero));


	for (i = 0; i < mob_n; i++) {
		uint8_t n = 0;
		if (rand() % 10 > 7 || i == 0) {
			uint8_t collided = 0;
			do {
				if (sprite_exists(&mob[i])) {
					sprite_destroy(&mob[i]);
				}
				mob_x[i] = rand() % LCD_X;
				mob_y[i] = rand() % LCD_Y;
				sprite_init(
					&mob[i],
					mob_x[i],
					mob_y[i],
					5, 6, mob_bitmap
				);

				for (n = 0; n < i; n++) {
					if (sprite_exists(&mob[n])) {
						collided = process_collision(&mob[n], &mob[i]);
						if (collided) {
							break;
						}
					}
				}
			} while (
				process_collision(&mob[i], &hero) ||
				process_collision(&mob[i], &door) ||
				collided
			);			
		} else {
			sprite_destroy(&mob[i]);
			sprite_visible(&mob[i], 0);
		}
	}

	uint8_t mob_collided = 0;
	do {
		sprite_destroy(&key);
		key_x = rand() % LCD_X;
		key_y = rand() % LCD_Y;
		sprite_init(
			&key,
			key_x,
			key_y,
			7, 3, key_bitmap
		);

		uint8_t n = 0;
		mob_collided = 0;
		for (n = 0; n < i; n++) {
			if (sprite_exists(&mob[n])) {
				mob_collided = process_collision(&key, &mob[n]);
				if (mob_collided) {
					break;
				}
			}
		}
	} while (
		process_collision(&key, &hero) ||
		process_collision(&key, &door) ||
		mob_collided
	);

	if ((rand() % 10) > 7) {
		do {
			if (sprite_exists(&shield)) {
				sprite_destroy(&shield);
			}
			shield_x = rand() % LCD_X;
			shield_y = rand() % LCD_Y;
			sprite_init(
				&shield,
				shield_x,
				shield_y,
				4, 4, shield_bitmap
			);

			uint8_t n = 0;
			mob_collided = 0;
			for (n = 0; n < i; n++) {
				if (sprite_exists(&mob[n])) {
					mob_collided = process_collision(&shield, &mob[n]);
					if (mob_collided) {
						break;
					}
				}
			}
		} while (
			process_collision(&shield, &hero) ||
			process_collision(&shield, &door) ||
			process_collision(&shield, &key) ||
			mob_collided
		);
	}

	for (i = 0; i < treasure_n; i++) {
		uint8_t n = 0;
		uint8_t collided = 0;
		if (rand() % 10 > 5) {
			do {
				if (sprite_exists(&treasure[i])) {
					sprite_destroy(&treasure[i]);
				}
				treasure_x[i] = rand() % LCD_X;
				treasure_y[i] = rand() % LCD_Y;
				sprite_init(
					&treasure[i],
					treasure_x[i],
					treasure_y[i],
					5, 3, treasure_bitmap
				);

				for (n = 0; n < i; n++) {
					if (sprite_exists(&treasure[n])) {
						collided = process_collision(&treasure[n], &treasure[i]);
						if (collided) {
							break;
						}
					}
				}
				
				mob_collided = 0;
				for (n = 0; n < i; n++) {
					if (sprite_exists(&mob[n])) {
						mob_collided = process_collision(&key, &mob[n]);
						if (mob_collided) {
							break;
						}
					}
				}

			} while(
				process_collision(&treasure[i], &hero) ||
				process_collision(&treasure[i], &door) ||
				process_collision(&treasure[i], &key) ||
				process_collision(&treasure[i], &shield) ||
				collided ||
				mob_collided
			);
		}	
	}
		
	// Walls
	sprite_destroy(&wall_left);
	sprite_destroy(&wall_top);
	sprite_destroy(&wall_right);
	sprite_destroy(&wall_bot);

	sprite_init(&wall_left, -40, -40, 3, 100, wall_left_bitmap);
	sprite_init(&wall_top, -40, -40, 160, 3, wall_top_bitmap);
	sprite_init(&wall_right, 120, -40, 3, 100, wall_left_bitmap);
	sprite_init(&wall_bot, -40, 60, 160, 3, wall_top_bitmap);

	if (level > 2) {
		for (i = 0; i < walls_n; i++) {
			sprite_destroy(&wall[i]);
		}
	}

	for (i = 0; i < walls_n; i++) {

		uint8_t t_width = i <= 2 ? 3 : 100 * 0.25;
		uint8_t t_height = i <= 2 ? 160 * 0.25 : 3;
		uint8_t t_collided = 0;
		uint8_t n;

		do {
			wall_x[i] = rand() % 130 - 35;
			wall_y[i] = rand() % 140 - 35;

			sprite_init(
				&wall[i],
				wall_x[i],
				wall_y[i],
				t_width,
				t_height,
				i <= 2 ? wall_left_bitmap : wall_top_bitmap
			);

			for (n = 0; n < i; n++) {
				t_collided = process_collision(&wall[i], &wall[n]);
			}

			uint8_t n = 0;
			uint8_t treasure_collided = 0;
			for (n = 0; n < i; n++) {
				if (sprite_exists(&treasure[n])) {
					treasure_collided = process_collision(&treasure[n], &treasure[i]);
					if (treasure_collided) {
						break;
					}
				}
			}
			
			mob_collided = 0;
			for (n = 0; n < i; n++) {
				if (sprite_exists(&mob[n])) {
					mob_collided = process_collision(&key, &mob[n]);
					if (mob_collided) {
						break;
					}
				}
			}

			t_collided = t_collided 
						|| process_collision(&wall[i], &hero) 
						|| process_collision(&wall[i], &key) 
						|| process_collision(&wall[i], &door)
						|| process_collision(&wall[i], &wall_left)
						|| process_collision(&wall[i], &wall_right)
						|| process_collision(&wall[i], &wall_top)
						|| process_collision(&wall[i], &wall_bot)
						|| treasure_collided
						|| mob_collided;

			if (t_collided) {
				sprite_destroy(&wall[i]);
			}

		} while (t_collided);
	}

	uint8_t q = 0;
	for (q = 0; q < mob_n; q++) {
		if (sprite_exists(&mob[q])) {
			sprite_set_speed(&mob[q], .25, .25);
		}
	}
	
	// sprite_set_speed(mob, 1, 1);
}

int sprite_find_x(void) {

	return 0;
}

// ---------------------------------------------------------
//	Timer overflow business.
// ---------------------------------------------------------

#define FREQ 8000000.0
#define PRESCALE 256.0
#define TIMER_SCALE 256.0

float interval = 0;

void send_data() {
	sprintf(message, "\r\n%d:%d%d", o_minutes, o_tenth_seconds, o_seconds);
	usb_serial_send(message);
	sprintf(message, "%d", score);
	usb_serial_send(" | Score: ");
	usb_serial_send(message);
	usb_serial_send(" | Floor: ");
	sprintf(message, "%d", level);
	usb_serial_send(message);
	usb_serial_send(" | Lives: ");
	sprintf(message, "%d", lives);
	usb_serial_send(message);
}

ISR(TIMER0_OVF_vect) {
	interval += TIMER_SCALE * PRESCALE / FREQ;

	srand((unsigned) (int)(interval * 10 * seconds * tenth_seconds));

	if ( interval >= 0.45 && interval <= 0.455) {
		if (level != 0) {
			send_data();
		}
	}

	if ( interval >= 0.97 ) {

		seconds += 1;
		if (seconds >= 10) {
			seconds = 0;
			tenth_seconds += 1;
		}
		if (tenth_seconds >= 6) {
			tenth_seconds = 0;
			minutes += 1;
		}	

		o_seconds += 1;
		if (o_seconds >= 10) {
			o_seconds = 0;
			o_tenth_seconds += 1;
		}
		if (o_tenth_seconds >= 6) {
			o_tenth_seconds = 0;
			o_minutes += 1;
		}	

		if (level != 0) {
			send_data();
		}

		// reset interval
		interval = 0;

		// Proof of life: toggle LED
		uint8_t current_bit = BIT_VALUE(PORTD, 6);
		WRITE_BIT(PORTD, 6, (1 - current_bit));
	}
}

void game_over_screen() {
	has_shield = 0;
	has_key = 0;

	clear_screen();

	sprite_destroy(&hero);
	sprite_destroy(&tower);
	sprite_destroy(&door);
	uint8_t q = 0;
	for (q = 0; q < mob_n; q++) {
		if (sprite_exists(&mob[q])) {
			sprite_destroy(&mob[q]);
		}
	}
	sprite_destroy(&key);

	draw_string(15, 0, "GAME OVER", FG_COLOUR);
	draw_string(5, 10, "Score: ", FG_COLOUR);
	draw_int(40, 10, score, FG_COLOUR);
	draw_string(5, 20, "Floor: ", FG_COLOUR);
	draw_int(40, 20, level - 1, FG_COLOUR);

	// Initialise sprites
	sprite_init(&hero, LCD_X / 2, LCD_Y / 2, 7, 10, hero_bitmap);
	sprite_init(&tower, 6, -10, 72, 26, tower_bitmap);
	sprite_init(&door, 30, 5, 24, 12, door_bitmap);
	sprite_init(&mob[0], 72, 20, 5, 6, mob_bitmap);
	sprite_init(&key, 6, 20, 7, 3, key_bitmap);

	// Initialise walls 
	sprite_init(&wall_left, -40, -40, 3, 100, wall_left_bitmap);
	sprite_init(&wall_top, -40, -40, 160, 3, wall_top_bitmap);
	sprite_init(&wall_right, 120, -40, 3, 100, wall_left_bitmap);
	sprite_init(&wall_bot, -40, 60, 160, 3, wall_top_bitmap);

	// Initialise sprite movements
	sprite_set_speed(&hero, hero_speed, hero_speed);
	sprite_set_speed(&mob[0], .25, .25);

	score = 0;
	lives = 3;
	level = 0;
	minutes = 0;
	tenth_seconds = 0;
	seconds = 0;

	show_screen();

	do {
		
	} while((!BIT_IS_SET(PINF, 5) && !BIT_IS_SET(PINF, 6)));

	clear_screen();
	draw_string(20, 22, "n9901990", FG_COLOUR);
	draw_string(6, 10, "Jia Sheng Chong", FG_COLOUR);
	show_screen();

	do {
		
	} while((!BIT_IS_SET(PINF, 5) && !BIT_IS_SET(PINF, 6)));	

}

void loading_screen() {
	clear_screen();
	draw_string(15, 10, "Loading...", FG_COLOUR);
	draw_string(15, 20, "Floor: ", FG_COLOUR);
	draw_int(52, 20, level, FG_COLOUR);
	
	show_screen();
	_delay_ms(2200);
	return;
}

void status_screen() {
	draw_string(5, 0, "Score: ", FG_COLOUR);
	draw_int(40, 0, score, FG_COLOUR);
	draw_string(5, 10, "Lives: ", FG_COLOUR);
	draw_int(40, 10, lives, FG_COLOUR);
	draw_string(5, 20, "Floor: ", FG_COLOUR);
	draw_int(40, 20, level - 1, FG_COLOUR);
	draw_string(5, 30, "Time: ", FG_COLOUR);
	draw_int(40, 30, minutes, FG_COLOUR);
	draw_string(45, 30, ":", FG_COLOUR);
	draw_int(50, 30, tenth_seconds, FG_COLOUR);
	draw_int(55, 30, seconds, FG_COLOUR);
}

void process() {

	// GUARD: Check if the start button has been pressed when level is 0
	if ((BIT_IS_SET(PINF, 5) || BIT_IS_SET(PINF, 6)) && level == 0) {
		level = 1;
		clear_screen();
		draw_string(31, 20, "- 3 -", FG_COLOUR);
		show_screen();
		_delay_ms(333);
		clear_screen();
		draw_string(31, 20, "- 2 -", FG_COLOUR);
		show_screen();
		_delay_ms(333);
		clear_screen();
		draw_string(31, 20, "- 1 -", FG_COLOUR);
		show_screen();
		_delay_ms(333);
		return;
	}

	// GUARD: Check if the level is still at 0
	if (level == 0) {
		draw_string(20, 22, "n9901990", FG_COLOUR);
		draw_string(6, 10, "Jia Sheng Chong", FG_COLOUR);
		show_screen();
		return;
	}

	// GUARD: Check if player looks at status screen
	if (BIT_IS_SET(PINB, 0) && level != 0) {
		status_screen();
		show_screen();
		return;
	}

	// Player movements
	// LEFT
	if (BIT_IS_SET(PINB, 1)) {
		last_direction = 'L';
		move_all('L');
	}
	else if (BIT_IS_SET(PINB, 7)) {
		last_direction = 'D';
		move_all('D');
	}
	else if (BIT_IS_SET(PIND, 0)) {
		last_direction = 'R';
		move_all('R');
	}
	else if (BIT_IS_SET(PIND, 1)) {
		last_direction = 'U';
		move_all('U');
	}

	//draw_line(-10, -10, -10, 50, 1);

	sprite_draw(&wall_left);
	sprite_draw(&wall_top);
	sprite_draw(&wall_right);
	sprite_draw(&wall_bot);
	sprite_draw(&hero);
	sprite_draw(&door);
	sprite_draw(&key);
	
	uint8_t q = 0;
	for (q = 0; q < mob_n; q++) {
		if (&mob[q] != NULL) {
			sprite_draw(&mob[q]);
			mob_move(&hero, &mob[q]);
		}
	}

	if (level == 1) {
		sprite_draw(&tower);
		process_collision(&hero, &tower);
		process_collision(&mob[0], &tower);
		
	}

	if (level > 1) {
		uint8_t i = 0;

		for (i = 0; i < mob_n; i++) {
			process_collision(&mob[i], &door);
		}
		
		for (i = 0; i < walls_n; i++) {
			sprite_draw(&wall[i]);
			process_collision(&hero, &wall[i]);
		}

		for (i = 0; i < treasure_n; i++) {
			if (sprite_is_visible(&treasure[i]) != 0) {
				sprite_draw(&treasure[i]);
				if (process_collision(&hero, &treasure[i])) {
					usb_serial_send("\r\nThe player picked up a treasure");
					sprite_destroy(&treasure[i]);
					score += 10;
				}
			}	
		}
	
	}

	// Check if the hero touches the key
	// And if the hero touches the key and touches the door
	// Move on to the next level
	if(process_collision(&hero, &key)) {
		sprite_follow(&key, &hero);

		if (!has_key) {
			usb_serial_send("\r\nThe player has the key!");
		}
		has_key = 1;

		if (process_collision(&hero, &door)) {
			usb_serial_send("\r\nThe player uses the key to the next floor");
			next_level();
		}
	}

	// Checks if the hero is touching the door without the key
	process_collision(&hero, &door);

	process_collision(&hero, &wall_left);
	process_collision(&hero, &wall_right);
	process_collision(&hero, &wall_top);
	process_collision(&hero, &wall_bot);

	// Check if the hero touches the monster
	if (sprite_exists(&shield)) {
		sprite_draw(&shield);

		if (process_collision(&hero, &shield)) {
			if (!has_shield) {
				usb_serial_send("\r\nPlayer picked up the shield");
			}
			has_shield = 1;

			sprite_follow_2(&shield, &hero);

			uint8_t collided = 0;
			uint8_t p = 0;
			for (p = 0; p < mob_n; p++) {
				if (sprite_exists(&mob[p])) {
					collided = process_collision(&hero, &mob[p]) || process_collision(&shield, &mob[p]);
				}
				if (collided) {
					break;
				}
			}

			if (collided) {	
				usb_serial_send("\r\nThe player killed a mosnter with a shield!");
				sprite_set_xy(&mob[p], 200, 200);
				sprite_set_xy(&shield, -200, -200);
				sprite_destroy(&mob[p]);
				sprite_destroy(&shield);
			}
		}
	}

	uint8_t b = 0;
	for (b = 0; b < mob_n; b++) {
		if (sprite_exists(&mob[b])) {
			if (process_collision(&hero, &mob[b])) {
				usb_serial_send("\r\nThe player was killed by the monster");
				sprite_destroy(&hero);
				restart_level();
			}
		}
	}

	show_screen();
}

int main(void) {
	setup();

	_delay_ms(1000);

	for ( ;; ) {
		process();
		_delay_ms(100);
		clear_screen();
	}

	return 0;
}
