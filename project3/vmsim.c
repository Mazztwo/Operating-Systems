//
//  CS 1550 Operating Systems
//  Alessio Mazzone
//  Project3
//


#include<stdio.h>
#include<sys/mman.h>
#include<stdlib.h>


// Tracefile
FILE *traceFile;

// Tracefile info
unsigned int address;
char mode;

// Total number of memory accesses
int memoryAccesses;

void OPT(int rawr)
{
    
}



int main(int argc, char* argv[])
{
    traceFile = fopen("gcc.trace", "r");
    
    if(!traceFile)
    {
        printf("ERROR: Trace file could not be opened. Please try again.\n");
        return 0;
    }
    
    
    // Read first line in trace file
    int scan = fscanf(traceFile, "%x %c", &address, &mode);
    
    
    // Initialize memoryAccesses to 0
    // Don't initialize to 1 because below, if reading line fails, we still increment
    // and so we account for that by initializing to 0 even though we did a memory access.
    memoryAccesses = 0;
    
    // Read every line in trace file
    while(scan > 1)
    {
        // Print line in trace file
        printf("%x %c\n", address, mode);
        
        // Read next line in trace file
        scan = fscanf(traceFile, "%x %c", &address, &mode);
        memoryAccesses += 1;
    }
    
    
    printf("Total memory accesses: %d\n", memoryAccesses);
    
    fclose(traceFile);
    return 0;
}




