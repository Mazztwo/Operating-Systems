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



int main(int argc, char* argv[])
{
    /*
        Producer code...
        1) Add item to buffer
        2) If buffer full, sleep
        3) If buffer = 1, means was empty before,
           so wake up consumer.
     
        Consumer code...
        1) Remove item from buffer
        2) If buffer empty, sleep
        3) If buffer = N-1, means was full before,
           so wake up producer.
     
        We need empty, full, and mutex semaphores.
        empty = number of empty spots
        full = number of full spots
        mutex = for mutual exclusion, initialize to 1
     
     
        SYSCALLS:
        down = 325
        up = 326
        syscall(325/326, sem);
     
     
    */
    
    
    // First we grab the command line arguments
    int numConsumers = atoi(argv[1]);
    int numProducers = atoi(argv[2]);
    int bufferSize = atoi(argv[3]);
    
    // Make shared buffer before fork() so memory can be shared
    void *buffer = mmap(NULL, bufferSize, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    
    
    
    return 0;
}




