/** @file movie.c
 * @brief Definition of the struct data and the functions prototypes for movie.h
 * 
 * 
 * @author Gonçalo Soares & Gonçalo Rodrigues
 * @date 9 May 2023
 * @bug No known bugs.
 */

#include <zephyr/sys/printk.h>      /* for printk()*/
#include "movie.h"

uint32_t id = 0;
static node *head = NULL;

void addMovie(const char *mname, uint32_t mprice, uint32_t mhours, uint32_t mminut){
    node *newNode = (node*)k_malloc(sizeof(node));
    newNode->movie.name = mname;
    newNode->movie.price = mprice;
    newNode->movie.hours = mhours;
    newNode->movie.minutes = mminut;
    newNode->movie.id = id; 
    id++;
    newNode->next = head;
    head = newNode;
}

void removeMovie(uint32_t id){
    node *temp = head;
    node *prev = NULL;
    while(temp != NULL){
        if(temp->movie.id == id){
            if(prev == NULL){
                head = temp->next;
            }
            else{
                prev->next = temp->next;
            }
            k_free(temp);
            return;
        }
        prev = temp;
        temp = temp->next;
    }
}

void printMovie(uint32_t id){
    node *temp = head;
    while(temp != NULL){
        if(temp->movie.id == id){
            printk("\t\t\t\t\t\t\tMovie: %s   |", temp->movie.name);
			printk("Price: %.2d   |", temp->movie.price);
			printk("Time: %d:%.2d\r", temp->movie.hours, temp->movie.minutes);

            break;
        }
        temp = temp->next;
    }
}

int sizeMovies(){
    node *temp = head;
    int size = 0;
    while(temp != NULL){
        size++;
        temp = temp->next;
    }
    return size;
}

movie returnMovie(uint32_t id){
    node *temp = head;
    while(temp != NULL){
        if(temp->movie.id == id){
            return temp->movie;
        }
        temp = temp->next;
    }
    return temp->movie;
}
