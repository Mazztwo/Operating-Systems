/*
	FUSE: Filesystem in Userspace
	Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

	This program can be distributed under the terms of the GNU GPL.
	See the file COPYING.
*/

#define	FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

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


// Returns index of directory in root. Returns -1 if not found
static int get_directory(char *directory)
{
    int dirIndex = -1;
    
    FILE *disk = fopen(".disk","rb");
    
    if(disk == NULL)
    {
        printf("get_directory-ERROR: .disk could not be opened!\n");
        return -1;
    }
    
    printf("get_directory-OPENED DISK!\n");
    
    cs1550_root_directory root;
    int r = fread(&root, sizeof(cs1550_root_directory), 1, disk);
    
    printf("get_directory-GOT ROOT!\n");
    fclose(disk);
    
    if(r <=0)
    {
        printf("get_directory-ERROR: Could not read root.\n");
        return -1;
    }
    
    // See if dir exists
    int i;
    for (i = 0; i < root.nDirectories; i++)
    {
        if (strcmp(root.directories[i].dname, directory) == 0)
        {
            dirIndex = i;
            break;
        }
    }
    
    return dirIndex;
}


// Returns size of file in directory. Returns -1 if not found
static size_t get_file_size(int dirIndex, char *file)
{
    // Have dir index
    // Get root.dir[index].start block
    // Seek to that block
    // Get dir.nfiles
    // go through and look for files
    
    size_t fileSize = -1;
    
    FILE *disk = fopen(".disk","rb");
    
    if(disk == NULL)
    {
        printf("get_file_size-ERROR: .disk could not be opened!\n");
        return -1;
    }
    
    printf("get_file_size-OPENED DISK!\n");
    
    cs1550_root_directory root;
    int r = fread(&root, sizeof(cs1550_root_directory), 1, disk);
    
    if(r <=0)
    {
        printf("get_file_size-ERROR: Could not read root.\n");
        fclose(disk);
        return -1;
    }
    
    printf("get_file_size-GOT ROOT!\n");
    
    // Get start block of parent
    long dirStart = root.directories[dirIndex].nStartBlock;
    
    // Seek to block and get directory
    cs1550_directory_entry parent;
    fseek(disk, dirStart, SEEK_SET);
    fread(&parent, BLOCK_SIZE, 1, disk);
    
    printf("get_file_size-GOT PARENT DIR!\n");
    
    // See if file exists
    int i;
    for (i = 0; i < parent.nFiles; i++)
    {
        if (strcmp(parent.files[i].fname, file) == 0)
        {
            fileSize = parent.files[i].fsize;
            break;
        }
    }
    
    fclose(disk);
    return fileSize;
}


// Checks if number of directories is max.
// Returns 0 if capacity reached
// Returns 1 if capacity not reached
static int check_dir_capacity()
{
    int cap = 0;
    
    FILE *disk = fopen(".disk","rb");
    
    if(disk == NULL)
    {
        printf("check_dir_capacity-ERROR: .disk could not be opened!\n");
        return 0;
    }
    
    printf("check_dir_capacity-OPENED DISK!\n");
    
    cs1550_root_directory root;
    int r = fread(&root, sizeof(cs1550_root_directory), 1, disk);
    
    printf("check_dir_capacity-GOT ROOT!\n");
    fclose(disk);
    
    if(r <=0)
    {
        printf("check_dir_capacity-ERROR: Could not read root.\n");
        return 0;
    }
    
    if(root.nDirectories >= MAX_DIRS_IN_ROOT)
    {
        cap = 0;
    }
    else
    {
        cap = 1;
    }
    
    
    return cap;
}

// Gets the next free block available on disk using the free space tracking at the end
// Returns 0 if free block could not be found
// Returns block number if free block is found
static int get_free_block(FILE *disk)
{
    int blockPos = 0;
    
    // create bitmap
    // seek to position
    // read bitmap
    // seek back to start of bit map
    unsigned char map[3*BLOCK_SIZE];
    fseek(disk, -3*BLOCK_SIZE, SEEK_END);
    fread(map, 3*BLOCK_SIZE, 1, disk);
    fseek(disk, -3*BLOCK_SIZE, SEEK_END);
    
    // Find next free block
    int i;
    //for(i = 0)
    
    
    
    
    
    return blockPos;
}



// Creates new dir at next available free block
// Returns 0 if dir could not be created
// Returns 1 if dir was created successfully
static int create_new_dir(char *directory)
{
    int created = 0;
    
    FILE *disk = fopen(".disk","r+b");
    
    if(disk == NULL)
    {
        printf("create_new_dir-ERROR: .disk could not be opened!\n");
        return 0;
    }
    
    printf("create_new_dir-OPENED DISK!\n");
    
    cs1550_root_directory root;
    int r = fread(&root, sizeof(cs1550_root_directory), 1, disk);
    
    printf("create_new_dir-GOT ROOT!\n");
    
    if(r <=0)
    {
        printf("create_new_dir-ERROR: Could not read root.\n");
        return 0;
    }
    
    
    int freeBlock = get_free_block(disk);
    
    // Seek back to beginning of file
    // !!!!!!
    
    if(freeBlock > 0 )
    {
        printf("create_new_dir-FREE BLOCK FOUND!\n");
        
        
        created = 1;
        
    }
    else
    {
        printf("create_new_dir-ERROR: COULD NOT FIND FREE BLOCK!\n");
        created = 0;
    }
    
    
    
    
    fclose(disk);
    return created;
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
    char directory[MAX_FILENAME+1];
    char filename[MAX_FILENAME+1];
    char extension[MAX_EXTENSION+1];
    
    memset(directory,  0,MAX_FILENAME + 1);
    memset(filename, 0,MAX_FILENAME  + 1);
    memset(extension,0,MAX_EXTENSION + 1);
	memset(stbuf, 0, sizeof(struct stat));
   
    printf("GET_ATTR!\n");
    
	//is path the root dir?
	if (strcmp(path, "/") == 0)
    {
        printf("PATH IS ROOT!\n");
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	}
    else
    {
        // Get info
        printf("SCANNING PATH!\n");
        sscanf(path, "/%[^/]/%[^.].%s", directory, filename, extension);
        printf("PATH: dir: %s, file: %s, ext: %s\n", directory, filename, extension);
        
        // Check if dir exists
        int dirIndex = get_directory(directory);
        printf("DIR_INDEX: %d\n", dirIndex);
        
        // Dir exists
        if(dirIndex > 0)
        {
            printf("DIRECTORY EXISTS!\n");
            // Check if file exists
            size_t fileSize = get_file_size(dirIndex, filename);
            printf("FILE_SIZE: %zu\n", fileSize);
            
            // File exists
            if(fileSize > 0)
            {
                printf("FILE EXISTS!\n");
                 //regular file, probably want to be read and write
                 stbuf->st_mode = S_IFREG | 0666;
                 stbuf->st_nlink = 1; //file links
                 stbuf->st_size = fileSize; //file size - make sure you replace with real size!
                 res = 0; // no error
            }
            // File does not exist
            else
            {
                printf("FILE DOES NOT EXIST!\n");
                //Might want to return a structure with these fields
                stbuf->st_mode = S_IFDIR | 0755;
                stbuf->st_nlink = 2;
                res = 0; //no error
            }
        }
        // Dir does not exist
        else
        {
            printf("ERROR: Directory specified does not exist.\n");
            res = -ENOENT;
        }
	}
    
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
    
    int res = 0;
    char directory[MAX_FILENAME+1];
    char filename[MAX_FILENAME+1];
    char extension[MAX_EXTENSION+1];
    
    memset(directory,  0,MAX_FILENAME + 1);
    memset(filename, 0,MAX_FILENAME  + 1);
    memset(extension,0,MAX_EXTENSION + 1);
    
    printf("MAKING_DIR!\n");
    
    // Get info
    printf("SCANNING PATH!\n");
    sscanf(path, "/%[^/]/%[^.].%s", directory, filename, extension);
    printf("PATH: dir: %s, file: %s, ext: %s\n", directory, filename, extension);
    
    /*
     0 on success
     ENAMETOOLONG if the name is beyond 8 chars
     EPERM if the directory is not under the root dir only
     EEXIST if the directory already exists
     */
    
    // Check dir name length
    if (strlen(directory) > 8)
    {
        printf("DIR NAME TOO LONG!\n");
        res = -ENAMETOOLONG;
    }
    // Make dir only under root
    else if (filename != NULL)
    {
        printf("TRIED TO MAKE DIR NOT IN ROOT!\n");
        res = -EPERM;
    }
    
    // Check if dir exists
    int dirIndex = get_directory(directory);
    printf("DIR_INDEX: %d\n", dirIndex);

    // Dir exists
    if(dirIndex >= 0)
    {
        printf("DIRECTORY ALREADY EXISTS!\n");
        res = -EEXIST;
        
    }
    // Dir does not exist. Make it
    else
    {
        int c = check_dir_capacity();
        
        if(c == 1)
        {
            printf("CREATING EMPTY DIR!\n");
            c = create_new_dir(directory);
            
            if(c == 0)
            {
                printf("COULD NOT CREATE DIR!\n");
                res = -1;
            }
            else
            {
                printf("NEW DIR CREATED!\n");
                res = 0;
            }
        }
        else
        {
            printf("REACHED DIR CAPACITY!\n");
            res = -1;
        }
    }
    
	return res;
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
