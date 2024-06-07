#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>



int directory_findname(struct unixfilesystem *fs, const char *name, int dirinumber, struct direntv6 *dirEnt) {

   // Allocate memory for the inode structure.
    struct inode *inp = malloc(sizeof(struct inode));
    if (inp == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for inode structure\n");
        return -1; 
    }

    // Retrieve the inode of the directory.
    if (inode_iget(fs, dirinumber, inp) == -1) {
        fprintf(stderr, "Error: Could not fetch the inode for inode number %d\n", dirinumber);
        free(inp);
        return -1; 
    }

    // Check if the inode represents a directory
    int is_directory = ((inp->i_mode & IFMT) == IFDIR);
    if (!is_directory) {
        fprintf(stderr, "Error: inode number %d is not a directory\n", dirinumber);
        free(inp);
        return -1;
    }

    // Get the total size of the directory
    int total_size = inode_getsize(inp);
    if (total_size <= 0) {
        fprintf(stderr, "Error: Invalid directory size for inode number %d\n", dirinumber);
        free(inp);
        return -1; 
    }

    // Calculate the number of blocks in the directory
    int blocks = (total_size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;

    // Allocate memory for a buffer to read directory blocks
    void *buff = malloc(DISKIMG_SECTOR_SIZE);
    if (buff == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for buffer\n");
        free(inp);
        return -1; 
    }

    int dirent_size = sizeof(struct direntv6);

    // Iterate over each block in the directory
    for (int i = 0; i < blocks; i++) {
        int valid_bytes = file_getblock(fs, dirinumber, i, buff); // Read the directory block
        if (valid_bytes == -1) {
            fprintf(stderr, "Error: Could not fetch block %d for inode number %d\n", i, dirinumber);
            free(inp);
            free(buff);
            return -1; 
        }

        // Calculate the number of directory entries in the block
        int dirents_in_block = valid_bytes / dirent_size;

        // Iterate over each directory entry in the block
        for (int j = 0; j < dirents_in_block; j++) {
            struct direntv6 *dirent = (struct direntv6 *)(buff + j * dirent_size); // Get the directory entry

            // Compare the directory entry name with the target name
            if (strcmp(dirent->d_name, name) == 0) {
                memcpy(dirEnt, dirent, dirent_size); // Copy the directory entry to dirEnt
                free(inp);
                free(buff);
                return 0;  // Success
            }
        }
    }

    // Clean up allocated memory
    fprintf(stderr, "Error: Directory entry with name '%s' not found\n", name);
    free(buff);
    free(inp);
    return -1; // Directory entry not found
}



