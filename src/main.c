/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>          /* for k_msleep() */
#include <zephyr/device.h>          /* for device_is_ready() and device structure */
#include <zephyr/devicetree.h>		/* for DT_NODELABEL() */
#include <zephyr/drivers/gpio.h>    /* for GPIO api*/
#include <zephyr/sys/printk.h>      /* for printk()*/
#include "../libs/movie.h"

/* Use a "big" sleep time to reduce CPU load (button detection int activated, not polled) */
#define SLEEP_TIME_MS   60*1000 

/* Variables */
#define COIN1    11
#define COIN2    12
#define COIN5    24
#define COIN10   25
#define RETURN    3
#define SELECT    4
#define DOWN     28
#define UP       29
#define NO_EVENT  0

/* States */
#define GETTING_COINS_ST 5
#define MOVIE_ST         6
#define BUY_ST           7

/* Events */
volatile int Event = 0;

/* Set the pins used for LED and buttons */
/* LED 1 and buttons 1-4 are the ones on board */
/* Buttons 5-8 are connected to pins labeled A0 ... A3 (gpio0 pins 3,4,28,29) */
#define LED1_PIN 13
const uint8_t buttons_pins[] = {COIN1,COIN2,COIN5,COIN10,RETURN,SELECT,DOWN,UP}; /* vector with pins where buttons are connected */

/* Get node ID for GPIO0, which has leds and buttons */ 
#define GPIO0_NODE DT_NODELABEL(gpio0)

/* Now get the device pointer for GPIO0 */
static const struct device * gpio0_dev = DEVICE_DT_GET(GPIO0_NODE);

/* Define a variable of type static struct gpio_callback, which will latter be used to install the callback
*  It defines e.g. which pin triggers the callback and the address of the function */
static struct gpio_callback button_cb_data;

/* Function that enable the interrupts */
void enableInterrupts(int pin){
	gpio_pin_interrupt_configure(gpio0_dev, pin, GPIO_INT_EDGE_TO_ACTIVE);
}

/* Function that disable the interrupts */
void disableInterrupts(int pin){
	gpio_pin_interrupt_configure(gpio0_dev, pin, GPIO_INT_DISABLE);
}

/* Define a callback function */
/* that is called when the button is pressed */
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	int button=0;

	/* Disable interrupts for all buttons in order to prevent the "Bouncing effect" */
	for(int i=0; i<sizeof(buttons_pins); i++){
		disableInterrupts(buttons_pins[i]);
	}

	/* Toggle led1 */
	gpio_pin_toggle(gpio0_dev,LED1_PIN);

	/* Read buttons */
	for(int i=0; i<sizeof(buttons_pins); i++){        
        if(BIT(buttons_pins[i]) & pins) {
            button = buttons_pins[i];
        }
    } 

	//printk("button = %d \n\r",button);

	switch (button){
		case COIN1:
			Event = COIN1;
			break;
		case COIN2:
			Event = COIN2;
			break;
		case COIN5:
			Event = COIN5;
			break;
		case COIN10:
			Event = COIN10;
			break;
		case RETURN:
			Event = RETURN;
			break;
		case SELECT:
			Event = SELECT;
			break;
		case DOWN:
			Event = DOWN;
			break;
		case UP:
			Event = UP;
			break;
		default:
			break;
	}

	//printk("Event = %d\n\r",Event);

	/* Enable interrupts for all buttons once again */
	for(int i=0; i<sizeof(buttons_pins); i++){
		enableInterrupts(buttons_pins[i]);
	}
}

/* The main function */
void main(void)
{
	int ret, i;
	uint32_t pinmask = 0; /* Mask for setting the pins that shall generate interrupts */
	

	/* Check if gpio0 device is ready */
	if (!device_is_ready(gpio0_dev)) {
		printk("Error: gpio0 device is not ready\n");
		return;
	} else {
		printk("Success: gpio0 device is ready\n");
	}

	/* Configure the GPIO pins - LED1 for output and buttons 1-4 + IOPINS 2,4,28 and 29 for input */
	ret = gpio_pin_configure(gpio0_dev,LED1_PIN, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error: gpio_pin_configure failed for led1, error:%d\n\r", ret);
		return;
	}

	for(i=0; i<sizeof(buttons_pins); i++) {
		ret = gpio_pin_configure(gpio0_dev, buttons_pins[i], GPIO_INPUT | GPIO_PULL_UP);
		if (ret < 0) {
			printk("Error: gpio_pin_configure failed for button %d/pin %d, error:%d\n\r", i+1,buttons_pins[i], ret);
			return;
		} else {
			printk("Success: gpio_pin_configure for button %d/pin %d\n\r", i+1,buttons_pins[i]);
		}
	}

	/* Configure the interrupt on the button's pin */
	for(i=0; i<sizeof(buttons_pins); i++) {
		ret = gpio_pin_interrupt_configure(gpio0_dev, buttons_pins[i], GPIO_INT_EDGE_TO_ACTIVE );
		if (ret < 0) {
			printk("Error: gpio_pin_interrupt_configure failed for button %d / pin %d, error:%d", i+1, buttons_pins[i], ret);
			return;
		}
	}

	/* HW init done!*/
	printk("All devices initialized sucesfully!\n\r");

	/* Initialize the static struct gpio_callback variable   */
	pinmask=0;
	for(i=0; i<sizeof(buttons_pins); i++) {
		pinmask |= BIT(buttons_pins[i]);
	}
    gpio_init_callback(&button_cb_data, button_pressed, pinmask); 	
	
	/* Add the callback function by calling gpio_add_callback()   */
	gpio_add_callback(gpio0_dev, &button_cb_data);

	/* Declarations */
	int credit = 0;
	//node *head = NULL;
	int movie_id = 0;
	int movie_size = 0;
	int state = GETTING_COINS_ST, next_state = state;
	
	/* Initial startup message */
	printk("\n------Movie Vending Machine------\n\r");
	printk("Hit buttons 1-8 (1-4 for inserting money, 5-8 to navigate through the contents...)\n\r");

	/* Movie Additions */
	/*addMovie("Movie A", 9, 19, 0);
	addMovie("Movie A", 11, 21, 0);
	addMovie("Movie A", 9, 23, 0);
	addMovie("Movie B", 10, 19, 0);
	addMovie("Movie B", 12, 21, 0);*/

	//movie_size = sizeMovies();

	while (1) {
		switch (state){
			case GETTING_COINS_ST:
				printk("credit = %d EUR\r", credit);
				
				switch(Event){
					case COIN1:           // Coin 1 inserted 
						credit+=1;
						break;
					case COIN2:           // Coin 2 inserted
						credit+=2;
						break;
					case COIN5:           // Coin 5 inserted
						credit+=5;
						break;
					case COIN10:          // Coin 10 inserted
						credit+=10;
						break;
					case NO_EVENT:        // No Coin inserted
						break;
					default:
						break;
				}

				//if (Event != NO_EVENT) printk("Credit: %d EUR\r", credit); (not work)
				
				/*if (Event == DOWN || Event == UP) next_state = MOVIE_ST;
				else if (Event == RETURN){
					//printk("%d EUR return",credit);
					credit = 0;
					next_state = GETTING_COINS_ST;
				}
				else next_state = GETTING_COINS_ST;*/

				Event = NO_EVENT;         // Reset Event
				
				break;

			case MOVIE_ST:
				switch(Event){
					case DOWN:            // Down button pressed
						if(movie_id == 0) {
							movie_id = movie_size-1;
						}
						else {
							movie_id--;
						}

						printMovie(movie_id);

						break;
					case UP:              // Up button pressed
						if(movie_id == movie_size-1) {
							movie_id = 0;
						}
						else {
							movie_id++;
						}

						printMovie(movie_id);
						break;
					case SELECT: 
						next_state = BUY_ST;
					default:
						Event = NO_EVENT; // Reset Event
				}
				Event = NO_EVENT;
				break; 
			
			case BUY_ST:
				break;

			default:
				break;
		}
		state = next_state;

	/* Just sleep. Led 
	on/off is done by the int callback */
	k_msleep(SLEEP_TIME_MS);
	
	}
}


