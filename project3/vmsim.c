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
    int location;
    unsigned int pageAddr;
    struct Node *next;
    struct Node *previous;
};


// Preprocess
// Preprocess map, one index for every memory address (20 bit address, 32-12bit offset)
struct Node *futureLocations[1048575];

void initFrames(struct Page *frames)
{
    int i;
    for(i = 0; i < numFrames; i++)
    {
        frames[i].address = 0;
        frames[i].dirty = 0;
        
        // Flipping the valid bit here is important. It is the main indicator that tells us
        // whether or not a frame in memory is free. If the valid bit = 1, it means there is
        // a valid page within the frame. If the valid bit is = 0, this means that the frame is
        // free for use.
        frames[i].valid = 0;
        frames[i].reference = 0;
    }
}

// Set the futureLocations heads to all point to the last item in each list,
// which is the first occurance of that address in the trace. Don't have to
// worry about losing references to rest of list because lists are doubly linked,
// so access to other elements is done with futureLocations[i]->previous
void futureLocationsToEnd(struct Node *futureLocations[])
{
    int i;
    for(i = 0; i < 1048575; i++)
    {
        if(futureLocations[i] != NULL)
        {
            while(futureLocations[i]->next != NULL)
                futureLocations[i] = futureLocations[i]->next;
        }
    }
}


// Returns the index of the first free frame in memory frames.
// If no free frame in memory, returns  -1.
int freeFrameIndex(struct Page frames[])
{
    int i;
    for(i = 0; i < numFrames; i++)
    {
        if(frames[i].valid == 0)
            return i;
    }
    return -1;
}


// Returns the index of a specific page in the memory frames.
// Returns the index of the page if found, or -1 if page not found.
int pageInFrames(struct Page frames[], unsigned int page)
{
    int i;
	for(i = 0; i < numFrames; i++)
	{
		if(frames[i].address == page)
			return i;
	}
	return -1;
}


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
    Page Fault ** Evict page that is used FURTHEST in the future **
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

// Optimum Page Replacement Algorithm
void opt()
{
    struct Page* frames = (struct Page*) malloc(sizeof(struct Page) * numFrames);
    
    
    // initialize all of "memory" to empty pages
    initFrames(frames);
    
    // Store beginning of file in pos so that after preprocessing
    // the pointer can be set back to the beginning.
    fpos_t pos;
    fgetpos(traceFile, &pos);
    
    // Location of where the address is in the trace
    int traceLocation = 0;
    
    // Read every line in trace file, and keep reading until end of file
    while(fscanf(traceFile, "%x %c", &address, &mode) != EOF)
    {
        // Print line in trace file
        //printf("%x %c\n", address, mode);
        
        // First Left 20 bits are address, right 12 bits are offset
        unsigned int currPage = address & 0xfffff000;
        int ind = currPage >> 12;
        
        

        // Initialize node to keep track of list of locations
        struct Node *newPage = malloc(sizeof(struct Node*));
        newPage->location = traceLocation;
        newPage->pageAddr = currPage;
        newPage->next = NULL;
        newPage->previous = NULL;
        
        // Add the newPage to futureLocations if there doesn't exist a list yet
        if(futureLocations[ind] == NULL)
        {
            futureLocations[ind] = newPage;
        }
        //Add  page to the head of index list if list exists
        else
        {
            newPage->next = futureLocations[ind];
            futureLocations[ind]->previous = newPage;
            futureLocations[ind] = newPage;
        }
        
        traceLocation += 1;
    }
    
    // Reset file pointer to beginning of file
    fsetpos(traceFile, &pos);
    //fclose(traceFile);
    //traceFile = fopen(fileName, "r");
    
    
    // Reset traceLocation
    traceLocation = 0;
    
    // Set list in futureLocations to point to first occurances
    // Read every line in trace file, and keep reading until end of file
    futureLocationsToEnd(futureLocations);
    
    while(fscanf(traceFile, "%x %c", &address, &mode) != EOF)
	{
        //printf("%x %c\n", address, mode);
        
        // First Left 20 bits are address
        unsigned int currPage = address & 0xfffff000;

        // Now that preprocessing is done, the first thing to be done is to see if the page from the trace
        // is already in our "memory" frames. If it is, then it's a hit.
        int pageIndex = pageInFrames(frames,currPage);
        
        // This means page hit!
        if(pageIndex >= 0)
        {
            // If we find a page and are writing to it, we must flip the dirty bit
            if(mode == 'W')
				frames[pageIndex].dirty = 1;
                
			printf("%x, hit\n", address);
        }
        else // This means page miss (page fault)!
        {
            // Create new page to put into Frames since page fault occured. This simulates a
            // disk read.
            struct Page pageFromDisk;
            if(mode == 'W')
            {
                pageFromDisk.dirty = 1;
            }
            else
            {
                pageFromDisk.dirty = 0;
            }
            pageFromDisk.address = currPage;
            
            // This page is going into the frames since we have a page fault, and thus we flip
            // the valid bit, indicating that the page is in memory.
            pageFromDisk.valid = 1;
            
            // If there is a miss, that means that a page is NOT in the frames.
            // The first thing we must do is put the page into the frames.
            //      Case 1: We can put the page into a free frame
            //      Case 2: We must evict a page to make room for new page (page furtherst in the future)
            int freeInd = freeFrameIndex(frames);
            
            // Put page in free frame
            if(freeInd >= 0)
            {
                //frames[freeInd].address = currPage;
                //frames[freeInd] = pageFromDisk;
                printf("%x, page fault – no eviction\n", address);
            }
            else // Evict a page and then place page in evicted spot
            {
                int i;
                int j = 0;
                int k = 0;
                for(i = 0; i < numFrames; i++)
                {
                    // Index of liked list in list of lists (from prescanning)
                    int index = frames[i].address >> 12;
                    
                    
                    // Go through specific linked list to find furthest instance
                    while(futureLocations[index] != NULL && futureLocations[index]->location < traceLocation)
                    {
                        // Use previous here because the dubly linked list, we move
                        // backwards from tail
                        futureLocations[index] = futureLocations[index]->previous;
                    }
                    
                    // if page is furtherst in future
                    if(futureLocations[index]->location > k)
                    {
                        k = futureLocations[index]->location;
                        j = i;
                        
                    }
                    else if(futureLocations[index] == NULL) // End of list is reached, no more accesses to that page
                    {
                        j = i;
                        break;
                    }
                    
                    
                }
                
                // Last thing to do is write to disk if the page is dirty
                if(frames[j].dirty == 1)
                {
                    diskWrites += 1;
                    printf("%x, page fault – evict dirty\n", address);
                }
                else // If page is clean, no need to overwrite page in disk because page in disk is most recent copy
                {
                    // No need to increment diskWrites since page on disk is most recent copy
                    printf("%x, page fault – evict clean\n", address);
                }
                
                // Put page in frames
                //frames[freeInd] = pageFromDisk;

            }
            
            // Increase number of page faults for later printing
            pageFaults += 1;
            
            // Put page in frames
            frames[freeInd] = pageFromDisk;
        }

        // Increment traceLocation to next line in trace file
        traceLocation += 1;
        
        // Increment number of accesses for printing later
        memoryAccesses += 1;
        
        
        
    }
    
    
    free(frames);
    
}













void displayResults()
{
    printf("\n%s\n", algorithm);
    printf("Number of frames:        %d\n",numFrames);
    printf("Total memory accesses:   %d\n", memoryAccesses);
    printf("Total page faults:       %d\n", pageFaults);
    printf("Total writes to disk:    %d\n\n", diskWrites);
}





// Clock algorithm
void clock()
{
    struct Page* frames = (struct Page*) malloc(sizeof(struct Page) * numFrames);
    
    
    // initialize all of "memory" to empty pages
    initFrames(frames);
    
    while(fscanf(traceFile, "%x %c", &address, &mode) != EOF)
    {
        
    }
    
    
    
    free(frames);
    
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
    fclose(traceFile);
    return 0;
}




