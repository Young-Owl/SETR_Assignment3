/** @file movie.h
 * @brief Definition of the struct data and the function prototype for FIFO
 * 
 * 
 * @author Gonçalo Soares & Gonçalo Rodrigues
 * @date 12 March 2023
 * @bug No known bugs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef MOVIE_H
#define MOVIE_H

#define FIFO_SIZE 10

/**
 * @brief This struture contains the info for one movie.
 *
 */ 
typedef struct movie{
	char name[20];      /**< Name of the movie */
	uint32_t price;     /**< Price of the movie */
	uint32_t time;      /**< Time of the movie */
} movie;

/**
 * @brief This structure works as a node for a linked list.
 * 
 */	
typedef struct node{
	struct movie movie;
	struct node *next;
} node;

/**
 * @brief This function adds a movie to the linked list.
 * 
 * @param  mname Name of the movie.
 * @param  mprice Price of the movie.
 * @param  mtime Time of the movie.
 */
void addMovie(char *mname, uint32_t mprice, uint32_t mtime);

/**
 * @brief This function removes a movie from the linked list.
 * 
 * @param mname Name of the movie.
 */
void removeMovie(char *mname);

/**
 * @brief This function goes through the linked list and prints 
 * 
 * @param fifo Pointer to the FIFO structure.
 * @return The value of the oldest element (high priority element) in the FIFO structure.
 * @exception If the FIFO is empty, the return value is equal to 9999. 
 */
uint32_t myFIFORemove(struct MYFIFO *fifo);

#endif