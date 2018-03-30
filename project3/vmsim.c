//
//  CS 1550 Operating Systems
//  Alessio Mazzone
//  Project3
//


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


// Tracefile info
FILE *traceFile;

unsigned int address;
char mode;

// Totals
int memoryAccesses = 0;
int pageFaults = 0;
int diskWrites = 0;

// Command line arguments
int numFrames;
char *algorithm;
char *fileName;
int refr;
int tau;



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
void opt()
{
    struct Page frames[numFrames];
    
    
    // Read every line in trace file, and keep reading until end of file
    while(fscanf(traceFile, "%x %c", &address, &mode) != EOF)
    {
        // Print line in trace file
        printf("%x %c\n", address, mode);
        
        // Read next line in trace file
        //scan = fscanf(traceFile, "%x %c", &address, &mode);
        memoryAccesses += 1;
    }
    
    
    fclose(traceFile);
    
}






void displayResults()
{
    printf("%s\n", algorithm);
    printf("Number of frames:        %d\n",numFrames);
    printf("Total memory accesses:   %d\n", memoryAccesses);
    printf("Total page faults:       %d\n", pageFaults);
    printf("Total writes to disk:    %d\n", diskWrites);
}








// Parses command line arguments
void parseCommandLine(char* argv[])
{
    numFrames = atoi(argv[2]);
    
    algorithm = argv[4];
    
    if(!strcmp(algorithm,"opt") || !strcmp(algorithm,"clock"))
    {
        fileName = argv[5];
    }
    else
    {
        if(!strcmp(algorithm,"aging"))
        {
            refr = atoi(argv[6]);
            fileName = argv[7];
        }
        else // algorithm = "work"
        {
            tau = atoi(argv[6]);
            fileName = argv[7];
        }
    }
    
    printf("Trace file name: %s\n", fileName);
}





















int main(int argc, char* argv[])
{
    
    // Parse command line arguments
    parseCommandLine(argv);
    
    // Open trace file
    traceFile = fopen(fileName, "r");
    if(!traceFile)
    {
        printf("ERROR: Trace file could not be opened. Please try again.\n");
        return 0;
    }
    
    
    // Call appropirate algorithm
    if(!strcmp(algorithm,"opt"))
    {
        opt();
    }
    else if(!strcmp(algorithm,"clock"))
    {
        
    }
    else if(!strcmp(algorithm,"aging"))
    {
        
    }
    else // algorithm = "work
    {
        
    }
    
    
    // Display results
    displayResults();
    
    return 0;
}




