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
    
    
    printf("hallow\n");
    
    fclose(traceFile);
    return 0;
}




