//////////////////////
// Alessio Mazzone  //
// CS 1550          //
// Project 4        //
//////////////////////

#define	FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

//size of a disk block
#define	BLOCK_SIZE 512

//we'll use 8.3 filenames
#define	MAX_FILENAME 8
#define	MAX_EXTENSION 3

//How many files can there be in one directory?
#define MAX_FILES_IN_DIR (BLOCK_SIZE - sizeof(int)) / ((MAX_FILENAME + 1) + (MAX_EXTENSION + 1) + sizeof(size_t) + sizeof(long))

//The attribute packed means to not align these things
struct cs1550_directory_entry
{
	int nFiles;	//How many files are in this directory.
				//Needs to be less than MAX_FILES_IN_DIR

	struct cs1550_file_directory
	{
		char fname[MAX_FILENAME + 1];	//filename (plus space for nul)
		char fext[MAX_EXTENSION + 1];	//extension (plus space for nul)
		size_t fsize;					//file size
		long nStartBlock;				//where the first block is on disk
	} __attribute__((packed)) files[MAX_FILES_IN_DIR];	//There is an array of these

	//This is some space to get this to be exactly the size of the disk block.
	//Don't use it for anything.  
	char padding[BLOCK_SIZE - MAX_FILES_IN_DIR * sizeof(struct cs1550_file_directory) - sizeof(int)];
} ;

typedef struct cs1550_root_directory cs1550_root_directory;

#define MAX_DIRS_IN_ROOT (BLOCK_SIZE - sizeof(int)) / ((MAX_FILENAME + 1) + sizeof(long))

struct cs1550_root_directory
{
	int nDirectories;	//How many subdirectories are in the root
						//Needs to be less than MAX_DIRS_IN_ROOT
	struct cs1550_directory
	{
		char dname[MAX_FILENAME + 1];	//directory name (plus space for nul)
		long nStartBlock;				//where the directory block is on disk
	} __attribute__((packed)) directories[MAX_DIRS_IN_ROOT];	//There is an array of these

	//This is some space to get this to be exactly the size of the disk block.
	//Don't use it for anything.  
	char padding[BLOCK_SIZE - MAX_DIRS_IN_ROOT * sizeof(struct cs1550_directory) - sizeof(int)];
} ;


typedef struct cs1550_directory_entry cs1550_directory_entry;

//How much data can one block hold?
#define	MAX_DATA_IN_BLOCK (BLOCK_SIZE - sizeof(long))

struct cs1550_disk_block
{
	//The next disk block, if needed. This is the next pointer in the linked 
	//allocation list
	long nNextBlock;

	//And all the rest of the space in the block can be used for actual data
	//storage.
	char data[MAX_DATA_IN_BLOCK];
};

typedef struct cs1550_disk_block cs1550_disk_block;







static int getDirIndex(char *directory)
{
    int ind = -1;
    
    // Pointer to start of disk. Open in read, binary mode.
    FILE *disk = fopen(".disk", "rb");
    
    // If there is an issue with disk, return ENOENT
    if(disk == NULL)
    {
        fclose(disk);
        printf("ERROR OPENING DISK\n");
        return-ENOENT;
    }
    
    // Make sure to seek to beginning of disk to start search!
    fseek(disk, 0, SEEK_SET);
    cs1550_root_directory root;
    int er = fread(&root, BLOCK_SIZE, 1, disk);
    
    if(er != 1)
    {
        fclose(disk);
        printf("COULD NOT READ ROOT\n");
        return-ENOENT;
    }
    else if(root.directories == NULL)
    {
        fclose(disk);
        printf("ROOT DIRECTORIES[] == NULL\n");
        return-ENOENT;
    }
    
 
    int i;
    for(i = 0; i < root.nDirectories; i++)
    {
        if( strcmp(directory, root.directories[i].dname) == 0 )
        {
            ind = i;
        }
    }
    
    // DONT FORGET TO CLOSE DISK!
    fclose(disk);
    return ind;
}

static int getFileIndex(int dirIndex,char *filename, size_t *fileSize)
{
    int ind = -1;
    
    // Pointer to start of disk. Open in read, binary mode.
    FILE *disk = fopen(".disk", "rb");
    
    // If there is an issue with disk, return ENOENT
    if(disk == NULL)
    {
        fclose(disk);
        printf("ERROR OPENING DISK\n");
        return-ENOENT;
    }
    
    // Make sure to seek to beginning of disk to start search!
    fseek(disk, 0, SEEK_SET);
    cs1550_root_directory root;
    int er = fread(&root, BLOCK_SIZE, 1, disk);
    
    if(er != 1)
    {
        fclose(disk);
        printf("COULD NOT READ ROOT\n");
        return-ENOENT;
    }
    else if(root.directories == NULL)
    {
        fclose(disk);
        printf("ROOT DIRECTORIES[] == NULL\n");
        return-ENOENT;
    }
    
    
    // Go to block location. Get start block of directory file is in!
    fseek(disk, root.directories[dirIndex].nStartBlock * BLOCK_SIZE, SEEK_SET);
    
    // Read block from disk
    cs1550_directory_entry  entry;
    int res = fread(&entry, BLOCK_SIZE, 1, disk);
    
    if(res != 1)
    {
        fclose(disk);
        printf("COULD NOT READ DIRECTORY FROM DISK\n");
        return-ENOENT;
    }
    else if(entry.files == NULL)
    {
        fclose(disk);
        printf("DIRECTORY FILES[] == NULL\n");
        return-ENOENT;
    }
    
    int i;
    for(i = 0; i < entry.nFiles; i++)
    {
        // compare filename to path/filename
        if(strcmp(filename, entry.files[i].fname) == 0)
        {
            ind = i;
            *fileSize = entry.files[i].fsize;
        }
        
    }
    
    
    // DONT FORGET TO CLOSE DISK!
    fclose(disk);
    return ind;
}









/*
 * Called whenever the system wants to know the file attributes, including
 * simply whether the file exists or not. 
 *
 * man -s 2 stat will show the fields of a stat structure
 */
static int cs1550_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
	memset(stbuf, 0, sizeof(struct stat));
    
   
    // Declare each of the three pieces. We are using 8.3 scheme,
    // which means file names and directories are in 8.3 format.
    // Leave one char for \0, so technically 9.4!
    //char *filename = calloc(9, sizeof(char));
    //char *extension = calloc(4, sizeof(char));
    //char *directory = calloc(9, sizeof(char));
    
    char directory[MAX_FILENAME+1];
    char extension[MAX_EXTENSION+1];
    char filename[MAX_FILENAME+1];
    
    memset(directory, 0, MAX_FILENAME + 1);
    memset(extension, 0, MAX_EXTENSION + 1);
    memset(filename, 0, MAX_FILENAME + 1);
    
	// is path the root dir?
	if (strcmp(path, "/") == 0)
    {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}
    else
    {
        // Split path into filename, extension, directory
        //int numVarsFilled = sscanf(path, "/%[^/]/%[^.].%s", directory, filename, extension);
        sscanf(path, "/%[^/]/%[^.].%s", directory, filename, extension);
/* Scrapping this scheme as I couldn't get it to work.
May be useful later.
        // There are three cases we are concerned with in terms of the
        // return value of sscanf(path). The return value tells us the number
        // of variables that the function has filled. So, if everything is
        // correct in the path, the return value can be -0,1,2, or 3. Every path
        // should include at least a directory, so numVarsFilled should at
        // least always be 1 (unless error). If path points to a directory, then numVarsFilled
        // = 1. If path points to a filename, then numVarsFilled = 3, because
        // 1 directory + 1 file name + 1 extension since every filename has
        
        // an extension in our 8.3 scheme.
*/
        int dirIndex = getDirIndex(directory);
        printf("Location of directory: %d\n", dirIndex);
        
        if(dirIndex == -1)
        {
            printf("ERROR FINDING DIRECTORY.");
            return -ENOENT;
        }
        
        // Case 1: path points to a directory
        //if(numVarsFilled == 1
        if(filename[0] == '\0')
        {
            printf("Path indicated directory with no file.\n");
            // We must see if the directory pointed to by path
            // exists in our file system. Since we are guaranteed a
            // two tier system, we can start from root and work down. This
            // is because only root has subdirectories, and subdirectories
            // only contain files, as per the project specifications.
            
            /*
            // Make sure to seek to beginning of disk to start search!
            fseek(disk, 0, SEEK_SET);
            cs1550_root_directory root;
            fread(&root, BLOCK_SIZE, 1, disk);
            
            // Scan blocks in .disk to see if entry exists
            int i;
            for(i = 0; i < root.nDirectories; i++)
            {
                if( strcmp(directory, root.directories[i].dname) == 0 )
                {
                     //Might want to return a structure with these fields
                     stbuf->st_mode = S_IFDIR | 0755;
                     stbuf->st_nlink = 2;
                     res = 0; //no error
                    break;
                }
                else
                {
                    // directory not found
                    res = -ENOENT;
                }
            }
             */
            
            
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            res = 0; //no error
            
        }
        // Case 2: path points to a file
        // Changed from numVarsFilled == 3 to >= 2 because there is
        // a chance that a file may not have an extension.
        //else if(numVarsFilled  >= 2)
        else
        {
            printf("Path indicated file within valid subdirectory.\n");
            
            // Pass pointer of file size to fileIndex so we can
            // trick the funciton into giving us two return values.
            size_t fileSize = 0;
            int fileIndex = getFileIndex(dirIndex,filename, &fileSize);
            printf("Location of file: %d\n", fileIndex);
            
            if(fileIndex == -1)
            {
                printf("ERROR FINDING FILE IN VALID DIRECTORY.");
                return -ENOENT;
            }
            
            //regular file, probably want to be read and write
            stbuf->st_mode = S_IFREG | 0666;
            stbuf->st_nlink = 1; //file links
            stbuf->st_size = fileSize; //file size - make sure you replace with real size!
            res = 0; // no error
        
            /*
            // If a file is found, we must find the block that
            // the file starts at. Start by finding the block
            // where the directory is located.
            fseek(disk, 0, SEEK_SET);
            cs1550_root_directory root;
            fread(&root, BLOCK_SIZE, 1, disk);
            
            // Scan blocks in .disk to see if entry exists
            int i;
            
            // Initialize where directoryBlock is.
            // directoryBlock will hold the offset from root.
            // Initialized to -11 because 11 is my favorite number.
            long directoryBlock = -11;
            
            for(i = 0; i < root.nDirectories; i++)
            {
                if( strcmp(directory, root.directories[i].dname) == 0 )
                {
                    directoryBlock = root.directories[i].nStartBlock;
                }
                else
                {
                    // directory not found
                    res = -ENOENT;
                }
            }
            
            // Directory not found
            if(directoryBlock == -11)
            {
                //free(filename);
                //free(extension);
                //free(directory);
                fclose(disk);
                return -ENOENT;
            }
            
            // Move disk pointer to the start location of the directory block
            fseek(disk, BLOCK_SIZE*directoryBlock, SEEK_SET);
            
            // Create a directory entry
            cs1550_directory_entry entry;
            fread(&entry, BLOCK_SIZE, 1, disk);
            
            // We must now scan through the files to see if the file
            // found in our path exists in this directory. This is very
            // similar to how we found our directory from root.
            
            long startOfFile = -11;
            
            for(i = 0; i < entry.nFiles; i++)
            {
                // compare filename to path/filename
                // compare extension to path/filename.extension
                if(strcmp(filename, entry.files[i].fname) == 0)
                {
                    if(strcmp(extension, entry.files[i].fext) == 0)
                    {
                        startOfFile = entry.files[i].nStartBlock;
                    }
                }
            }
            
            // File was not found scaning directory
            if(startOfFile == -11)
            {
                res = -ENOENT;
            }
            // File was found in directory
            else
            {
                
                //regular file, probably want to be read and write
                stbuf->st_mode = S_IFREG | 0666;
                stbuf->st_nlink = 1; //file links
                stbuf->st_size = entry.files[startOfFile].fsize; //file size - make sure you replace with real size!
                res = 0; // no error
                
            }*/
                       
        }
        // Case 3: "In the case of an input failure before any data could be successfully interpreted, EOF is returned." Also, if no variables
        // could be filled, then this is also an error.
        /*
        else if(numVarsFilled == EOF || numVarsFilled == 0)
        {
                res = -ENOENT;
        }*/

	}
    
    
    // DONT FORGET TO FREE MALLOC'D PIECES (filename, directory, extension)
    // DONT FORGET TO CLOSE DISK FILE
    //free(filename);
    //free(extension);
    //free(directory);
    //fclose(disk);
    
	return res;
}

/* 
 * Called whenever the contents of a directory are desired. Could be from an 'ls'
 * or could even be when a user hits TAB to do autocompletion
 */
static int cs1550_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	//Since we're building with -Wall (all warnings reported) we need
	//to "use" every parameter, so let's just cast them to void to
	//satisfy the compiler
	(void) offset;
	(void) fi;

	//This line assumes we have no subdirectories, need to change
	if (strcmp(path, "/") != 0)
	return -ENOENT;

	//the filler function allows us to add entries to the listing
	//read the fuse.h file for a description (in the ../include dir)
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	/*
	//add the user stuff (subdirs or files)
	//the +1 skips the leading '/' on the filenames
	filler(buf, newpath + 1, NULL, 0);
	*/
	return 0;
}










/* 
 * Creates a directory. We can ignore mode since we're not dealing with
 * permissions, as long as getattr returns appropriate ones for us.
 */
static int cs1550_mkdir(const char *path, mode_t mode)
{
	(void) mode;

    
    // Pointer to start of disk. Open in read, binary mode.
    FILE *disk = fopen(".disk", "r+b");
    
    // If there is an issue with disk, return ENOENT
    if(disk == NULL)
    {
        fclose(disk);
        return-ENOENT;
    }
    
    // Declare each of the three pieces. We are using 8.3 scheme,
    // which means file names and directories are in 8.3 format.
    // Leave one char for \0, so technically 9.4!
    char *filename = calloc(9, sizeof(char));
    char *extension = calloc(4, sizeof(char));
    char *directory = calloc(9, sizeof(char));
    
    
    // Split path into filename, extension, directory
    int numVarsFilled = sscanf(path, "/%[^/]/%[^.].%s", directory, filename, extension);
    
    /*
     
     Return values:
     
     0 on success
     ENAMETOOLONG if the name is beyond 8 chars
     EPERM if the directory is not under the root dir only
     EEXIST if the directory already exists
     
     */
    
    // First we can check if the directory name is longer than 8 characters
    if(strlen(directory) > 8)
    {
        return -ENAMETOOLONG;
    }
    // Check if just root is given
    else if(strcmp(path, "/") == 0)
    {
        return -EEXIST;
    }
    // If there is more than 1 variable filled by sscanf, user trying to make
    // a new directory outside of root which goes against project speficications
    else if(numVarsFilled > 1)
    {
        return -EPERM;
    }
    // Make sure we weren't given just / as a directory
    else if(filename[0] == '/')
    {
        return -EPERM;
    }
    // Directory name is valid, no filename or extension given. From above else if
    // we can see that numVarsFilled is not greater than 1, which means no file name
    // was given.
    else if(directory[0] != 0)
    {
        // First let's check to make sure that the directory doesn't already exist
        // in our file system.
        // Make sure to seek to beginning of disk to start search!
        fseek(disk, 0, SEEK_SET);
        cs1550_root_directory root;
        fread(&root, BLOCK_SIZE, 1, disk);
        
        int numDirectories = root.nDirectories;
        
        // Scan blocks in .disk to see if entry exists
        int i;
        for(i = 0; i < numDirectories; i++)
        {
            if( strcmp(directory, root.directories[i].dname) == 0 )
            {
                return -EEXIST;
            }
        }
        
        // If we are here, then directory does not exist, so we must
        // update the root block with new entry information
 
        long start;
        
        if(numDirectories == 0)
        {
            start = BLOCK_SIZE;
        }
        else
        {
            start = BLOCK_SIZE * numDirectories;
        }
            
        strcpy(root.directories[numDirectories].dname, directory);
        root.directories[numDirectories].nStartBlock = start;
        root.nDirectories += 1;
        
        // Update root block
        fseek(disk, 0, SEEK_SET);
        fwrite(&root, BLOCK_SIZE, 1, disk);
        
        // We must write the new entry at the appropirate block in disk
        // Create entry to store in disk
        cs1550_directory_entry entry;
        
        // Move disk pointer to the start location of the directory block
        fseek(disk, start, SEEK_SET);
        fwrite(&entry, BLOCK_SIZE, 1, disk);
    }
    
    // DON'T FORGET TO FREE ALL CALLOC'D MEMORY
    // AND CLOSE ALL FILES!
    fclose(disk);
    free(filename);
    free(extension);
    free(directory);
    
	return 0;
}







/* 
 * Removes a directory.
 */
static int cs1550_rmdir(const char *path)
{
	(void) path;
    return 0;
}

/* 
 * Does the actual creation of a file. Mode and dev can be ignored.
 *
 */
static int cs1550_mknod(const char *path, mode_t mode, dev_t dev)
{
	(void) mode;
	(void) dev;
    
    
    // Remove later:
    (void) path;
	return 0;
}

/*
 * Deletes a file
 */
static int cs1550_unlink(const char *path)
{
    (void) path;

    return 0;
}

/* 
 * Read size bytes from file into buf starting from offset
 *
 */
static int cs1550_read(const char *path, char *buf, size_t size, off_t offset,
			  struct fuse_file_info *fi)
{
	(void) buf;
	(void) offset;
	(void) fi;
	(void) path;

	//check to make sure path exists
	//check that size is > 0
	//check that offset is <= to the file size
	//read in data
	//set size and return, or error

	size = 0;

	return size;
}

/* 
 * Write size bytes from buf into file starting from offset
 *
 */
static int cs1550_write(const char *path, const char *buf, size_t size, 
			  off_t offset, struct fuse_file_info *fi)
{
	(void) buf;
	(void) offset;
	(void) fi;
	(void) path;

	//check to make sure path exists
	//check that size is > 0
	//check that offset is <= to the file size
	//write data
	//set size (should be same as input) and return, or error

	return size;
}

/******************************************************************************
 *
 *  DO NOT MODIFY ANYTHING BELOW THIS LINE
 *
 *****************************************************************************/

/*
 * truncate is called when a new file is created (with a 0 size) or when an
 * existing file is made shorter. We're not handling deleting files or 
 * truncating existing ones, so all we need to do here is to initialize
 * the appropriate directory entry.
 *
 */
static int cs1550_truncate(const char *path, off_t size)
{
	(void) path;
	(void) size;

    return 0;
}


/* 
 * Called when we open a file
 *
 */
static int cs1550_open(const char *path, struct fuse_file_info *fi)
{
	(void) path;
	(void) fi;
    /*
        //if we can't find the desired file, return an error
        return -ENOENT;
    */

    //It's not really necessary for this project to anything in open

    /* We're not going to worry about permissions for this project, but 
	   if we were and we don't have them to the file we should return an error

        return -EACCES;
    */

    return 0; //success!
}

/*
 * Called when close is called on a file descriptor, but because it might
 * have been dup'ed, this isn't a guarantee we won't ever need the file 
 * again. For us, return success simply to avoid the unimplemented error
 * in the debug log.
 */
static int cs1550_flush (const char *path , struct fuse_file_info *fi)
{
	(void) path;
	(void) fi;

	return 0; //success!
}


//register our new functions as the implementations of the syscalls
static struct fuse_operations hello_oper = {
    .getattr	= cs1550_getattr,
    .readdir	= cs1550_readdir,
    .mkdir	= cs1550_mkdir,
	.rmdir = cs1550_rmdir,
    .read	= cs1550_read,
    .write	= cs1550_write,
	.mknod	= cs1550_mknod,
	.unlink = cs1550_unlink,
	.truncate = cs1550_truncate,
	.flush = cs1550_flush,
	.open	= cs1550_open,
};

//Don't change this.
int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &hello_oper, NULL);
}
