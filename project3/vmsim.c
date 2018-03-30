//
//  CS 1550 Operating Systems
//  Alessio Mazzone
//  Project3
//


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


// Tracefile
FILE *traceFile;

// Tracefile info
unsigned int address;
char mode;

// Total number of memory accesses
int memoryAccesses;

int numFrames = 0;






/*
 # frames = max number of addresses allowed to be stored
 OPT:
 At start, we have number of frames, and a trace of memory accesses (simulating a process)
 We visit the first address in trace.
    If it is a read, dirtyBit = 0, if write, dirtyBit = 1.
 We must then search our frames for that address
    If page is there
        Hit
    If page is not there
        Page Fault ** Evict page that is used FURTHEST in the future
            If free frame is available
                Page Fault - No Eviction
            If free frame is not available
                Page Fault - evict clean/dirty
    Move to next line in trace
 
 PREPROCESSING:
    In order to know which page to evict on page fault, we must
    know the address in currently in the frames that is used FURTHEST in the future.
    That's the one we evict.
 
 
 
 
 
 
 
*/


/* Create a struct that simulates a page

*/
struct Page
{
    int dirty;
    int valid;
};
// To initialize: sturct Page p1; p1.clean = 0, p1.dirty = 1


// Optimum Page Replacement Algorithm
void OPT(FILE *traceFile)
{
    struct Page frames[5];
    
    
    // Initialize memoryAccesses
    memoryAccesses = 0;
    
    // Read every line in trace file, and keep reading until end of file
    while(fscanf(traceFile, "%x %c", &address, &mode) != EOF)
    {
        // Print line in trace file
        printf("%x %c\n", address, mode);
        
        // Read next line in trace file
        //scan = fscanf(traceFile, "%x %c", &address, &mode);
        memoryAccesses += 1;
    }
    
    printf("Total memory accesses: %d\n", memoryAccesses);
    
    fclose(traceFile);
    
}



int main(int argc, char* argv[])
{
    traceFile = fopen("gcc.trace", "r");
    
    if(!traceFile)
    {
        printf("ERROR: Trace file could not be opened. Please try again.\n");
        return 0;
    }
    
    
    
    // Parse arguments
    numFrames = atoi(argv[2]);
    printf("Number of Frames: %d\n",numFrames);
    

    
   
    
    
    
    return 0;
}




