/** @file main.c
 * @brief Main file for the Movie Vending Machine.
 * 
 * This file contains all the code for the Movie Vending machine.
 * It uses 4 internal buttons and 4 external buttons to navigate through the contents,
 * having a "not optimal" way to minimize the bouncing effect that was almost constant.
 * 
 * Along with Movie selection, an option to buy Popcorn was implemented. The code was
 * implemented after making a state machine diagram, used as a base to write the code for
 * the events and states thought previously.
 * 
 * 
 * @author Gonçalo Soares & Gonçalo Rodrigues
 * @date 10 May 2023
 * @bug No known bugs.
 */


#include <zephyr/kernel.h>          /* for k_msleep() */
#include <zephyr/device.h>          /* for device_is_ready() and device structure */
#include <zephyr/devicetree.h>		/* for DT_NODELABEL() */
#include <zephyr/drivers/gpio.h>    /* for GPIO api*/
#include <zephyr/sys/printk.h>      /* for printk()*/
#include "../libs/movie.h"

/* Use a "big" sleep time to reduce CPU load (button detection int activated, not polled) */
#define SLEEP_TIME_MS   60*1000 

/* Events */
#define COIN1    11	/**< Add 1 EUR to the credit via Button 1, Pin 11. 			*/
#define COIN2    12	/**< Add 2 EUR to the credit via Button 2, Pin 12. 			*/
#define COIN5    24	/**< Add 5 EUR to the credit via Button 3, Pin 24. 			*/
#define COIN10   25	/**< Add 10 EUR to the credit via Button 4, Pin 25. 		*/
#define RETURN    3	/**< Button to return the credit, Pin 3.					*/
#define SELECT    4	/**< Button to select, Pin 4.								*/
#define DOWN     28	/**< Navigation button down, Pin 28.						*/
#define UP       29	/**< Navigation button up, Pin 29.							*/
#define NO_EVENT  0	/**< Variable to prevent infinite loops within the states.	*/

/* States */
#define GETTING_COINS_ST 5	/**< State where the user inserts coins. 							*/
#define MOVIE_ST         6	/**< State where the user selects the movie.						*/
#define BUY_ST           7	/**< State where the user buys the ticket. 							*/
#define POPCORN_ST       8	/**< State where the user selects the amount of popcorn they want. 	*/

/* Events */
volatile int Event = 0;	/**< Variable to store the event that ocurred. (Button Interrupt)	*/

/* Set the pins used for LED and buttons */
/* LED 1 and buttons 1-4 are the ones on board */
/* Buttons 5-8 are connected to pins labeled A0 ... A3 (gpio0 pins 3,4,28,29) */
#define LED1_PIN 13	/* Debug purposes */
const uint8_t buttons_pins[] = {COIN1,COIN2,COIN5,COIN10,RETURN,SELECT,DOWN,UP}; /* Vector with pins where buttons are connected */

/* Get node ID for GPIO0, which has leds and buttons */ 
#define GPIO0_NODE DT_NODELABEL(gpio0)

/* Now get the device pointer for GPIO0 */
static const struct device * gpio0_dev = DEVICE_DT_GET(GPIO0_NODE);

/* Define a variable of type static struct gpio_callback, which will latter be used to install the callback
*  It defines e.g. which pin triggers the callback and the address of the function */
static struct gpio_callback button_cb_data;

/* Function that enables the interrupts */
void enableInterrupts(int pin){
	gpio_pin_interrupt_configure(gpio0_dev, pin, GPIO_INT_EDGE_TO_ACTIVE);
}

/* Function that disables the interrupts */
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
			Event = NO_EVENT;
			break;
	}

	/* Enable interrupts for all buttons once again */
	for(int i=0; i<sizeof(buttons_pins); i++){
		enableInterrupts(buttons_pins[i]);
	}
}

/*!
\dot
digraph finite_state_machine {
	rankdir=LR;
    nodesep=0.;
	size="10"

	node [shape = doublecircle]; GETTING_COINS_ST
	node [shape = circle];

    GETTING_COINS_ST -> GETTING_COINS_ST [ label = "                                                    COIN1/credit+=1"];
    GETTING_COINS_ST -> GETTING_COINS_ST [ label = "                                                    COIN2/credit+=2" ];
    GETTING_COINS_ST -> GETTING_COINS_ST [ label = "                                                    COIN5/credit+=5" ];
    GETTING_COINS_ST -> GETTING_COINS_ST [ label = "                                                        COIN10/credit+=10" ];
    GETTING_COINS_ST -> GETTING_COINS_ST [ label = "                                                     RETURN/credit=0" ];
    GETTING_COINS_ST -> GETTING_COINS_ST [ label = "                                            NO_EVENT/" ];
	GETTING_COINS_ST -> GETTING_COINS_ST [ label = "                                            RETURN/(credit = 0,popcornFlag = 0, popcorn = 0, sumAll = 0)" ];
	GETTING_COINS_ST -> MOVIE_ST [ label = "UP[popcornFlag = 0]/" ];
    GETTING_COINS_ST -> MOVIE_ST [ label = "DOWN[popcornFlag = 0]/" ];

    MOVIE_ST -> POPCORN_ST [ label = "SELECT/"];
    MOVIE_ST -> GETTING_COINS_ST;

    POPCORN_ST -> BUY_ST [ label = "SELECT/"];
    POPCORN_ST -> GETTING_COINS_ST [ label = "/sumAll = infoMovie.price + popcorn*2"];

    GETTING_COINS_ST -> POPCORN_ST [ label = "UP[popcornFlag = 1]/" ];
    GETTING_COINS_ST -> POPCORN_ST [ label = "DOWN[popcornFlag = 1]/" ];
	GETTING_COINS_ST -> POPCORN_ST [ label = "SELECT/popcornFlag = 1" ];

    BUY_ST -> GETTING_COINS_ST [ label = "/ popcornFlag = 0"];
	GETTING_COINS_ST -> BUY_ST [ label = "SELECT[popcornFlag = 1]/" ];

}
\enddot
*/

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

	/* Variables to store all the information needed */
	int credit = 0, movie_id = 0, movie_size = 0, popcorn = 0, sumAll = 0, popcornFlag = 0;
	int state = GETTING_COINS_ST, next_state = state;
	movie infoMovie;
	
	/* Initial startup message */
	printk("\n------Movie Vending Machine------\n\r");
	printk("Hit buttons 1-8 (1-4 for inserting money, 5-8 to navigate through the contents...)\n\r");

	/* Movie Additions */
	addMovie("Movie A", 9, 19, 0);
	addMovie("Movie A", 11, 21, 0);
	addMovie("Movie A", 9, 23, 0);
	addMovie("Movie B", 10, 19, 0);
	addMovie("Movie B", 12, 21, 0);

	/* Get the size of the linked list (Dynamic)*/
	movie_size = sizeMovies();

	printk("\n");
	while (1) {
		switch (state){
			case GETTING_COINS_ST:	
				printk("Credit = %.3d EUR\t Basket: %.3d EUR\r", credit,sumAll);

				switch(Event){
					case COIN1:           // Coin 1 inserted 
						credit+=1;
						Event = NO_EVENT;         // Reset Event
						break;
					case COIN2:           // Coin 2 inserted
						credit+=2;
						Event = NO_EVENT;         // Reset Event
						break;
					case COIN5:           // Coin 5 inserted
						credit+=5;
						Event = NO_EVENT;         // Reset Event
						break;
					case COIN10:          // Coin 10 inserted
						credit+=10;
						Event = NO_EVENT;         // Reset Event
						break;
					case DOWN:			  // Down button pressed
						if(popcornFlag == 1){
							next_state = POPCORN_ST;
						} 
						else next_state = MOVIE_ST;
						break;
					case UP:  			  // Up button pressed
						if(popcornFlag == 1){
				 	    	next_state = POPCORN_ST;
						}
						else next_state = MOVIE_ST;
						break;
					case SELECT:		  // Select button pressed
						if(popcornFlag == 1){
							next_state = BUY_ST;
						}
						else{
							printf("\n");
							next_state = POPCORN_ST;
							popcornFlag = 1;	
						} 
						break;
					case RETURN: 		  // Return button pressed
						if(credit == 0) { printk("\nNo credit to return.");}
						if(popcornFlag ==1) { printk("\nOperation canceled, %.4d EUR return.\n", credit);}
						printk("\n%.4d EUR return.\n",credit);
						printk("\n");
						popcornFlag = 0;
						credit = 0;
						sumAll = 0;
						popcorn = 0;
						Event = NO_EVENT;
						break;
					case NO_EVENT:        // No Coin inserted
						break;
					default:
						break;
				}
				break;

			/* Movie State goes after the Getting Coins State */
			case MOVIE_ST:
				switch(Event){
					case DOWN:            // Down button pressed
						if(movie_id == 0){
					    	movie_id = movie_size-1;
						}	
						else movie_id--;
						next_state = GETTING_COINS_ST;
						break;
					case UP:              // Up button pressed
						if(movie_id == movie_size-1){
							movie_id = 0;
						} 
						else movie_id++;
						next_state = GETTING_COINS_ST;
						break;
					default:
						Event = NO_EVENT; // Reset Event
						break;
				}
				infoMovie = returnMovie(movie_id);

				printMovie(movie_id);

				Event = NO_EVENT;
				break; 
			
			/* Popcorn State goes after the Movie State */
			case POPCORN_ST:
				popcornFlag = 1;
				printk("\t\t\t\t\t\t\tPopcorn Quantity: ");
				switch(Event){
					case DOWN:            // Down button pressed
						popcorn --;
						if(popcorn < 0){
							popcorn = 0;
						}	
						next_state = GETTING_COINS_ST;
						break;
					case UP:              // Up button pressed
						popcorn++;
						if(popcorn > 10){
							popcorn = 10;
						} 	
						next_state = GETTING_COINS_ST;
						break;
					default:
						Event = NO_EVENT; // Reset Event
						break;
				}
				
				printk("\t%.2d (Price: %.2d EUR)\r", popcorn, popcorn*2);
				Event = NO_EVENT;
				sumAll = infoMovie.price + popcorn*2;
				break;

			/* Buy State goes after Movie and Popcorn States */
			case BUY_ST:
				infoMovie = returnMovie(movie_id);
				printk("\n");
				if (credit < sumAll){
					printk("Not enough credit. Ticket not issued.\n");
					popcorn = 0;
					sumAll = 0;
					next_state = GETTING_COINS_ST;
				}
				else{
					printk("Ticket for movie %s, session %d:%.2d and %d popcorn issued.",infoMovie.name, infoMovie.hours, infoMovie.minutes, popcorn);
					credit -= sumAll;
					printk(" Remaining credit: %.2d EUR\n", credit);
					popcorn = 0;
					sumAll = 0;
					next_state = GETTING_COINS_ST;
				}
				printk("\n");
				Event = NO_EVENT;
				popcornFlag = 0;
				break;

			default:
				Event = NO_EVENT;
				break;
		}
		state = next_state;
	}

	/* Just sleep. Led ON/OFF is done by the int callback */
	k_msleep(SLEEP_TIME_MS);
}


