/** @file movie.c
 * @brief Definition of the struct data and the functions prototypes for movie.h
 * 
 * 
 * @author Gonçalo Soares & Gonçalo Rodrigues
 * @date 12 March 2023
 * @bug No known bugs.
 */

#include <zephyr/sys/printk.h>      /* for printk()*/
#include "movie.h"

uint32_t id = 0;
node *head = NULL;

void addMovie(char *mname, uint32_t mprice, uint32_t mtime){
    node *newNode = (node*) malloc(sizeof(node));
    strcpy(newNode->movie.name, mname);
    newNode->movie.price = mprice;
    newNode->movie.time = mtime;
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
            free(temp);
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
            printk("--------------------\n");
            printk("Name: %s\n", temp->movie.name);
            printk("Price: %d\n", temp->movie.price);
            printk("Time: %d\n", temp->movie.time);
            printk("--------------------\n");
            return;
        }
        temp = temp->next;
    }
}