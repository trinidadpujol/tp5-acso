#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "inode.h"
#include "diskimg.h"



int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {

    // Allocate memory for a sector buffer
    void *buffer = malloc(DISKIMG_SECTOR_SIZE);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Out of memory\n");
        return -1; 
    }
    int inodes_per_sector = DISKIMG_SECTOR_SIZE / sizeof(struct inode);  // Calculate the number of inodes per sector (256)
    int adjusted_inumber = inumber - 1; // Adjust inumber to be zero-based (index starts at zero)

    int sector_number = INODE_START_SECTOR + adjusted_inumber / inodes_per_sector;
    
    // Read the sector from disk
    if (diskimg_readsector(fs->dfd, sector_number, buffer) == -1) {
        free(buffer);   // if it fails, clean buffer 
        fprintf(stderr, "Error: Reading sector failed\n");
        return -1; 
    }

    // Calculate the offset in the sector for the inode we want
    int inode_offset = (adjusted_inumber % inodes_per_sector) * sizeof(struct inode);

    // Copy the inode from the buffer to the inode struct
    memcpy(inp, buffer + inode_offset, sizeof(struct inode));

    free(buffer);
    return 0;
}


int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) { 

    // Determine if the file is small 
    int is_small_file = ((inp->i_mode & ILARG) == 0);

    if (is_small_file) {
        // For small files, return the direct block if blockNum is < 7
        if (blockNum < 7) {
            return inp->i_addr[blockNum];
        } else {
            fprintf(stderr, "Error: Invalid block number for small files\n");
            return -1; // Invalid block number for small files
        }
    }

    // For large files handle indirect blocks
    const int blocks_per_sector = DISKIMG_SECTOR_SIZE / sizeof(uint16_t); // 256 blocks
    const int first_indir_limit = blocks_per_sector * 7; // 1792 blocks

    uint16_t *buffer_1 = malloc(DISKIMG_SECTOR_SIZE);  // Allocate memory for the first level indirect block buffer
    if (buffer_1 == NULL) {
        fprintf(stderr, "Error: Out of memory\n");
        return -1; 
    }

    // First level of indirection
    if (blockNum < first_indir_limit) {
        int indir_index = blockNum / blocks_per_sector; // Index of the indirect block.
        int offset = blockNum % blocks_per_sector; // Offset within the indirect block.

        // Read the sector containing the indirect block.
        if (diskimg_readsector(fs->dfd, inp->i_addr[indir_index], buffer_1) == -1) {
            free(buffer_1); // If it fails clean buffer 1
            fprintf(stderr, "Error: Reading sector failed\n");
            return -1; 
        }

        int block_number = buffer_1[offset];
        free(buffer_1);
        return block_number;
    }

    // Second level of indirection
    if (diskimg_readsector(fs->dfd, inp->i_addr[7], buffer_1) == -1) {
        free(buffer_1);
        fprintf(stderr, "Error: Reading sector failed\n");
        return -1; // First level indirect block read failed
    }

    uint16_t *buffer_2 = malloc(DISKIMG_SECTOR_SIZE); // Allocate mem for the second level indirect block buffer
    if (buffer_2 == NULL) {
        free(buffer_1);
        fprintf(stderr, "Error: Out of memory\n");
        return -1; 
    }

    int second_indir_index = (blockNum - first_indir_limit) / blocks_per_sector; // Index of the second level indirect block
    if (diskimg_readsector(fs->dfd, buffer_1[second_indir_index], buffer_2) == -1) {
        free(buffer_1);
        free(buffer_2);
        fprintf(stderr, "Error: Reading sector failed\n");
        return -1; // If it fails, clean buffer 1 and 2
    }

    int offset = (blockNum - first_indir_limit) % blocks_per_sector; // Offset within the second level indirect block
    int block_number = buffer_2[offset];

    free(buffer_1);
    free(buffer_2);
    return block_number;
} 

        

int inode_getsize(struct inode *inp) {
    return ((inp->i_size0 << 16) | inp->i_size1); 
}
