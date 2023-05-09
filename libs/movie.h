/** @file movie.h
 * @brief Definition of the struct data and the functions prototypes the Movie List.
 * 
 * 
 * @author Gonçalo Soares & Gonçalo Rodrigues
 * @date 9 May 2023
 * @bug No known bugs.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#ifndef MOVIE_H
#define MOVIE_H

/**
 * @brief This struture contains the info for one movie.
 *
 */ 
typedef struct movie{
	const char *name;   /**< Name of the movie */
	uint32_t price;     /**< Price of the movie */
	uint32_t hours;     /**< Hours of the movie */
	uint32_t minutes;   /**< Minutes of the movie */
	uint32_t id;		/**< ID of the movie */
} movie;

/**
 * @brief This structure works as a node for a linked list.
 * 
 */	
typedef struct node{
	movie movie;
	struct node *next;
} node;

/**
 * @brief This function adds a movie to the linked list.
 * 
 * @param  mname Name of the movie.
 * @param  mprice Price of the movie.
 * @param  mhours Hours of the movie.
 * @param  mminutes Minutes of the movie.
 */
void addMovie(const char *mname, uint32_t mprice, uint32_t mhours, uint32_t mminutes);

/**
 * @brief This function removes a movie from the linked list.
 * 
 * @param id ID of the movie.
 */
void removeMovie(uint32_t id);

/**
 * @brief This function prints a movie from the linked list.
 * 
 * @param id ID of the movie.
 */
void printMovie(uint32_t id);

/**
 * @brief This function returns the size of the movie list.
 * 
 */
int sizeMovies();

/**
 * @brief This function returns a movie from the linked list.
 * 
 */
movie returnMovie(uint32_t id);

#endif