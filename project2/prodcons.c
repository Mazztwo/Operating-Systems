//
//  CS 1550 Operating Systems
//  Alessio Mazzone
//  Project2
//


#include<stdio.h>
#include<sys/mman.h>
#include<stdlib.h>


// Semaphore struct
struct cs1550_sem
{
    int value;
    struct node* head;
    struct node* tail;
};


// To make calling our syscalls more natural in C
void down(cs1550_sem *sem)
{
    syscall(__NR_cs1550_down, sem);
}

void up(cs1550_sem *sem)
{
    syscall(__NR_cs1550_up, sem);
}


int main(int argc, char* argv[])
{

    
    
    
    return 0;
}




