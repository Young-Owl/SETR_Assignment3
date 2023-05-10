# nRF52840 Movie Vending Machine 

Repository for SETR's Assignment 3, where the objective was to implement a Movie Vending Machine using nRF52840 (Zephyr OS) after carefully planning a state machine for said tasks.

## IO

Uses 4 internal buttons within nRF52840, along with 4 external buttons. Respectively to simulate adding coins (1, 2, 5, 10 EUR) and the navigation buttons (UP, DOWN, SELECT, RETURN).

All button readings are via interrupts (Callback function). The buttons had "Bouncing" problems, making it difficult to interact with the machine, so a way to minimize this effect was implemented. This method involves the previous disabling of the interrupts within the callback function, enabling them before leaving it.

## State Machine

UML Design, with 4 different states for the needed steps through the interaction process. 