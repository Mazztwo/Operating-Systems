//
//  CS 1550 Operating Systems
//  Alessio Mazzone
//  Project2
//


#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>


// Semaphore struct
struct cs1550_sem
{
    int value;
    struct node* head;
    struct node* tail;
};


