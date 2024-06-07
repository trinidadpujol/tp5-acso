#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"



int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {

    // Allocate memory for the inode struct
    struct inode *inp = malloc(sizeof(struct inode));
    if (inp == NULL) {
        fprintf(stderr, "Error: Out of memory\n");
        return -1; 
    }

    // Retrieve the inode of the file
    if (inode_iget(fs, inumber, inp) == -1) {
        free(inp);
        fprintf(stderr, "Error: Could not fetch the specified inode from the filesystem\n");
        return -1; 
    }

    // Get the sector number corresponding to the block number
    int sector = inode_indexlookup(fs, inp, blockNum);
    if (sector == -1) {
        free(inp);
        fprintf(stderr, "Error: could not get the disk block number\n");
        return -1; 
    }

    // Read the sector from the disk into the buffer
    if (diskimg_readsector(fs->dfd, sector, buf) == -1) {
        free(inp);
        fprintf(stderr, "Error: could not get the number of bytes read from the sector\n");
        return -1; 
    }

    // Get the total size of the file
    int total_size = inode_getsize(inp);
    if (total_size <= 0) {
        free(inp);
        fprintf(stderr, "Error: could not get the total size of the file\n");
        return -1; 
    }

    free(inp); // Free the allocated inode struct

    // Calculate the number of blocks in the file
    int blocks = total_size / DISKIMG_SECTOR_SIZE;

    // Determine the size of the block to return
    if (blockNum < blocks) {
        return DISKIMG_SECTOR_SIZE; // Full size for non-last block.
    } else {
        return total_size % DISKIMG_SECTOR_SIZE; // Remaining size for last block.
    }
}

