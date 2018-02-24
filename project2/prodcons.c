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
    // Buffer includes in and out + the buffer size
    int *buffer = (int*) mmap(NULL, (2+bufferSize)*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    
    // Now initialize empty, full, and mutex
    // I'm not really sure about whether or not the "buffer size" specified in the command line
    // is a hard limit, and we can mmap a buffersize + sizeof(struct cs1550_sem)*3, or if we have to
    // make two separate buffers, one being the producer/consumer buffer and the other being the shared
    // semaphore buffer. I'll go with making two separate buffers just in case the directions
    // mean that our shared buffer is exactly the size specified in the command line, no more, no less.
    void *semaphoreSpace = mmap(NULL, sizeof(struct cs1550_sem)*3, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);
    
    struct cs1550_sem* empty;
    struct cs1550_sem* full;
    struct cs1550_sem* mutex;
    
    /* Map of semaphore_space:
    
     |__empty__|__full__|__mutex__|
     |    0    |   1    |    2    |
     
    */
    empty = (struct cs1550_sem*)semaphoreSpace;
    full = (struct cs1550_sem*)semaphoreSpace + 1;
    mutex = (struct cs1550_sem*)semaphoreSpace + 2;
    
    // Initial number of empty spaces is the whole buffer
    empty->value = bufferSize;
    
    // There are no spots filled initially
    full->value = 0;
    
    // Initialize mutex to 1 (unlocked)
    mutex->value = 1;
    
    // In and out
    int *in;
    int *out;
    
    // First two items in shared buffer are in and out
    in = buffer;
    out = buffer + 1;
    *in = 0;
    *out = 0;
    
    // Sett all buffer heads and tails to NULL
    empty->head = NULL;
    empty->tail = NULL;
    full->head = NULL;
    full->tail = NULL;
    mutex->head = NULL;
    mutex->tail = NULL;
    
    // Need to keep track of current buffer position
    // Initially right after in and out
    int *currBufferPosition;
    currBufferPosition = buffer + 2;

    // Variable used later to make sure parent waits for all children to finish
    int status;
    
    // Producers
    int i;
    
    for(i = 0; i < numProducers; i++)
    {
        // Keep creating child processes if not parent.
        // As per lecture slides, child PID = 0, while
        // parent is something greater than 0.
        if(fork() == 0)
        {
            int pitem;
            
            while(1)
            {
                // Down on empty, down on mutex
                syscall(325, empty);
                syscall(325, mutex);
                
                // Produce an item into pitem
                pitem = *in;
                
                // Add item to shared buffer
                currBufferPosition[*in] = pitem;
                printf("Chef %c Produced: Pancake%d\n", i+65, pitem);
                
                // Increment
                *in = (*in+1)%bufferSize;
                
                
                // Up on mutex, up on full
                syscall(326, mutex);
                syscall(326, full);
        
            }
        }

    }
    
    
    // Consumers
    for(i = 0; i < numConsumers; i++)
    {
        // Keep creating child processes if not parent.
        // As per lecture slides, child PID = 0, while
        // parent is something greater than 0.
        if(fork() == 0)
        {
            int citem;
            
            while(1)
            {
                // Down on full, down on mutex
                syscall(325, full);
                syscall(325, mutex);
                
               
                // Get item from buffer and "consume"
                citem = currBufferPosition[*out];
                printf("Customer %c Consumed: Pancake%d\n", i+65, citem);
                
                // Increment
                *out = (*out+1)%bufferSize;
    
                
                // Up on mutex, down on full
                syscall(326, mutex);
                syscall(326, empty);
                
                
            }
        }
    }
    
    
    // Wait here for all children to finish
    wait(&status);
    
    return 0;
}




