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

/* Use a "big" sleep time to reduce CPU load (button detection int activated, not polled) */
#define SLEEP_TIME_MS   60*1000 

/* Set the pins used for buttons */
/* Buttons 5-8 are connected to pins labeled A0 ... A3 (gpio0 pins 3,4,28,29) */

#define COIN1 11
#define COIN2 12
#define COIN5 24
#define COIN10 25
#define RETURN 3
#define SELECT 4
#define DOWN 28
#define UP 29

const uint8_t buttons_pins[] = {11,12,24,25,3,4,28,29}; /* vector with pins where buttons are connected */

/* Defining all global variables necessary for the Movie Vending Machine */
/* Struct for the Movie Info */
struct movie{
	char name[20];
	uint32_t price;
	uint32_t time;
};

/* Struct for the Linked List containing all movies */	
struct node{
	struct movie movie;
	struct node *next;
};

volatile uint32_t credit = 0;
volatile uint32_t coinAccept = 0;

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

	/* Identify the button(s) that was(ere) hit via "Switch-Case" */
	switch(buttons_pins[0]){
		case COIN1:
			/* 1 EUR - 1st Internal Button */
			printk("Introduced: 1 EUR \n\r");
			credit += 1;
			break;
		case COIN2:
			/* 2 EUR - 2nd Internal Button */
			printk("Introduced: 2 EUR \n\r");
			break;
		case COIN5:
			/* 5 EUR - 3rd Internal Button */
			printk("Introduced: 5 EUR \n\r");
			break;
		case COIN10:
			/* 10 EUR - 4th Internal Button */
			printk("Introduced: 10 EUR \n\r");
			break;
		case RETURN:
			/* RETURN Button */
			printk("Returned: %d EUR \n\r",1);
			break;
		case SELECT:
			/* SELECT Button*/
			printk("Selected: Something\n\r");
			break;
		case DOWN:
			/* DOWN Button */
			break;
		case UP:
			/* UP Button */
			break;
		default:
			printk("Error:\tButton not found\n\r");
			break;
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

	while (1) {
		/* Just sleep. Led on/off is done by the int callback */
		k_msleep(SLEEP_TIME_MS);
	}
}
