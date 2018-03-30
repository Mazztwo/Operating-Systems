//
//  CS 1550 Operating Systems
//  Alessio Mazzone
//  Project3
//


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>



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

// opt_preprocess






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
 
    To prescan:
        At start, read in address
 
 
 
 
 
 
*/


// Create a struct that simulates a page
struct Page
{
    unsigned int address;
    int dirty;
    int valid;
    int reference;
};// To initialize: sturct Page p1; p1.clean = 0, p1.dirty = 1


// Node for preprocess of OPT
struct Node
{
    int index;
    struct Node *next;
    struct Node *previous;
};

// Preprocess map, one index for every memory address
struct Node futureLocations[4294967295];



void initFrames(struct Page frames[])
{
    int i;
    for(i = 0; i < numFrames; i++)
    {
        frames[i].address = 0;
        frames[i].dirty = 0;
        frames[i].valid = 0;
        frames[i].reference = 0;
    }
}








// Optimum Page Replacement Algorithm
void opt()
{
    struct Page frames[numFrames];
    
    // initialize all of "memory" to empty pages
    initFrames(frames);
    
    // Store beginning of file in pos so that after preprocessing
    // the pointer can be set back to the beginning.
    fpos_t pos;
    fgetpos(traceFile, &pos);
    
    // Location of where the address is in the trace
    int traceLocation = 0;
    
    // Preprocess
    // Read every line in trace file, and keep reading until end of file
    
    while(fscanf(traceFile, "%x %c", &address, &mode) != EOF)
    {
        // Print line in trace file
        printf("%x %c\n", address, mode);
        
        //futureLocations[address] =
        
        // Read next line in trace file
        //scan = fscanf(traceFile, "%x %c", &address, &mode);
        memoryAccesses += 1;
    }
    
    // Reset file pointer to beginning of file
    fsetpos(traceFile, &pos);
    
    fclose(traceFile);
    
}









void displayResults()
{
    printf("\n%s\n", algorithm);
    printf("Number of frames:        %d\n",numFrames);
    printf("Total memory accesses:   %d\n", memoryAccesses);
    printf("Total page faults:       %d\n", pageFaults);
    printf("Total writes to disk:    %d\n\n", diskWrites);
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




