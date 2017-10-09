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

uint8_t level = 0;
uint8_t hero_speed = 2;
char last_direction;

// Initialise sprites
sprite_id hero;
sprite_id tower;
sprite_id door;
sprite_id mob;
sprite_id key;
sprite_id treasure;

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
	0b01000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010,
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

void sprite_set_speed(sprite_id sprite, float dx, float dy);
sprite_id sprite_create(float x, float y, uint8_t width, uint8_t height, uint8_t bitmap[]);

void setup() {
	uint8_t contrast = 175;

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
    CLEAR_BIT(DDRF, 6); // SW1 ?

	// Initialise backlight
	SET_BIT(DDRC, 7);

	// Initialise contrast
	lcd_init(contrast);

	// Initialise sprites
	hero = sprite_create(40, 30, 7, 10, hero_bitmap);
	tower = sprite_create(6, 1, 72, 16, tower_bitmap);
	door = sprite_create(30, 7, 24, 12, door_bitmap);
	mob = sprite_create(72, 20, 5, 6, mob_bitmap);
	key = sprite_create(6, 20, 7, 3, key_bitmap);
	sprite_set_speed(hero, hero_speed, hero_speed);
	sprite_set_speed(mob, 1, 1);

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

sprite_id sprite_create(float x, float y, uint8_t width, uint8_t height, uint8_t bitmap[]) {
	sprite_id sprite = malloc( sizeof( Sprite ) );

	if ( sprite != NULL ) {
		sprite_init( sprite, x, y, width, height, bitmap );
	}

	return sprite;
}

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
	sprite_move_all(tower, direction);
	sprite_move_all(mob, direction);
	sprite_move_all(key, direction);
	sprite_move_all(door, direction);	
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

void mob_move(sprite_id hero, sprite_id mob) {
	if (!sprite_is_visible(hero)) {
		return;
	}

	int hx = sprite_x(hero);
	int hy = sprite_y(hero);
	int mx = sprite_x(mob);
	int my = sprite_y(mob);

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

uint8_t sprite_width( sprite_id sprite ) {
	return sprite->width;
}

uint8_t sprite_height( sprite_id sprite ) {
	return sprite->height;
}

void sprite_destroy( sprite_id sprite ) {
	free(sprite);
}

int process_collision(sprite_id obj_1, sprite_id obj_2) {

	// Get platform and bird screen locations.
	uint8_t hx = round(sprite_x(obj_1)), hy = round(sprite_y(obj_1));
	uint8_t ox = round(sprite_x(obj_2)), oy = round(sprite_y(obj_2));

	// Check for collision
	int collided = 1;

	if ( hx >= ox + sprite_width(obj_2)) collided = 0;
	if ( hy >= oy + sprite_height(obj_2)) collided = 0;
	if ( ox >= hx + sprite_width(obj_1)) collided = 0;
	if ( oy >= hy + sprite_height(obj_1)) collided = 0;

	if (!collided || obj_2 == key) {
		return collided;
	}

	if (obj_1 == hero) {
		sprite_back_all();
	} else {
		sprite_move_to(hero, sprite_x(hero), sprite_y(obj_2) + sprite_height(obj_2));
	}

  return collided;
}


/**
 * DRIVERS 
 */

void next_level() {
	clear_screen();
	level += 1;
	if (tower != NULL) {
		sprite_destroy(tower);
	}
	if (treasure != NULL) {
		sprite_destroy(treasure);
	}
	sprite_destroy(hero);
	sprite_destroy(mob);
	sprite_destroy(key);

	hero = sprite_create(40, 30, 7, 10, hero_bitmap);
	sprite_set_speed(hero, 2, 2);
	// sprite_set_speed(mob, 1, 1);
	
}

void process() {
	clear_screen();

	// GUARD: Check if the start button has been pressed when level is 0
	if (BIT_IS_SET(PINF, 5) && level == 0) {
		level = 1;
		return;
	}

	// GUARD: Check if the level is still at 0
	if (level == 0) {
		draw_string(14, 23, "n9901990", FG_COLOUR);
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

	if (level == 1) {
		sprite_draw(hero);
		sprite_draw(tower);
		sprite_draw(door);
		sprite_draw(mob);
		sprite_draw(key);
		// mob_move(hero, mob);

		process_collision(hero, tower);
		process_collision(mob, tower);

		if (process_collision(hero, mob)) {
			// sprite_visible(hero, 0);
			sprite_destroy(hero);
		}

		if(process_collision(hero, key)) {
			sprite_follow(key, hero);

			if (process_collision(hero, door)) {
				next_level();
			}
		}
		
	}
	
	if (level > 1) {
		sprite_draw(hero);
	}

	show_screen();
}

int main(void) {
	setup();

	for ( ;; ) {
		process();
		_delay_ms(100);
	}

	return 0;
}
