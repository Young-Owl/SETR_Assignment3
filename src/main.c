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

/*Events*/
#define COIN1    11
#define COIN2    12
#define COIN5    24
#define COIN10   25
#define RETURN    3
#define SELECT    4
#define DOWN     28
#define UP       29
#define NO_EVENT  0

/*States*/
#define GETTING_COINS_ST 5
#define MOVIE_ST         6
#define BUY_ST           7

/*Variables*/
volatile int Event = 0; 


/* Set the pins used for buttons */
/* Buttons 5-8 are connected to pins labeled A0 ... A3 (gpio0 pins 3,4,28,29) */
const uint8_t buttons_pins[] = {COIN1,COIN2,COIN5,COIN10,RETURN,SELECT,DOWN,UP}; /* vector with pins where buttons are connected */

/* Get node ID for GPIO0, which has leds and buttons */ 
#define GPIO0_NODE DT_NODELABEL(gpio0)

/* Now get the device pointer for GPIO0 */
static const struct device * gpio0_dev = DEVICE_DT_GET(GPIO0_NODE);

/* Define a variable of type static struct gpio_callback, which will latter be used to install the callback
*  It defines e.g. which pin triggers the callback and the address of the function */
static struct gpio_callback button_cb_data;

void enableInterrupts(int pin){
	// Enable interrupts for the pin
	gpio_pin_interrupt_configure(gpio0_dev, pin, GPIO_INT_EDGE_TO_ACTIVE);
}

void disableInterrupts(int pin){
	// Disable interrupts for the pin
	gpio_pin_interrupt_configure(gpio0_dev, pin, GPIO_INT_DISABLE);
}

/* Define a callback function */
/* that is called when the button is pressed */
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	/* Disable interrupts for all buttons in order to prevent the "Bouncing effect" */
	for(int i=0; i<sizeof(buttons_pins); i++){
		disableInterrupts(buttons_pins[i]);
	}

	/*Read buttons*/
	for(int i=0; i<sizeof(buttons_pins); i++){        
        if(BIT(buttons_pins[i]) & pins) {
            Event = buttons_pins[i];
        }
    } 

	/* Enable interrupts for all buttons once again */
	for(int i=0; i<sizeof(buttons_pins); i++){
		enableInterrupts(buttons_pins[i]);
	}
}

void main(void)
{
	int ret, i;
	uint32_t pinmask = 0; /* Mask for setting the pins that shall generate interrupts */
	
	/* Initial startup message */
	printk("------Movie Vending Machine------\n\r");
	printk("Hit buttons 1-8 (1-4 for inserting money, 5-8 to navigate through the contents...\n\r");

	/* Check if gpio0 device is ready */
	if (!device_is_ready(gpio0_dev)) {
		printk("Error: gpio0 device is not ready\n");
		return;
	} else {
		printk("Success: gpio0 device is ready\n");
	}

	/* Configure the GPIO pins - IOPINS 2,4,28 and 29 for input */
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
	int state = GETTING_COINS_ST, next_state = state;
	int state_coins = NO_EVENT;

	while (1) {
		
		// State Machine
		switch(state){
			case GETTING_COINS_ST:
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
					default:
						Event = NO_EVENT; // Reset Event
				}
				Event = NO_EVENT;         // Reset Event
				 
				if (Event == DOWN || Event == UP) next_state = MOVIE_ST;
				else next_state = GETTING_COINS_ST;
				
				break;
			
			case MOVIE_ST:
				break; 

			case BUY_ST:
				break;

			default:
				break;	
			state = next_state;
		}

	}

	/* Just sleep. Led on/off is done by the int callback */
	k_msleep(SLEEP_TIME_MS);
	
}

