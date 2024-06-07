
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>


int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {

    // Duplicate the pathname to avoid modifying the original string
    char *pathname_copy = strdup(pathname);
    if (pathname_copy == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for pathname copy\n");
        return -1;
    }

    // Tokenize the pathname to split by "/" and get the first directory
    char *directory = strtok(pathname_copy, "/");
    int inumber = 1; // Start at the root directory

    // Allocate memory for a directory entry structure
    struct direntv6 *dirent = malloc(sizeof(struct direntv6));
    if (dirent == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for directory entry\n");
        free(pathname_copy);
        return -1;
    }

    // Iterate over each directory in the pathname
    while (directory != NULL) {
        // Look up the directory entry by name
        if (directory_findname(fs, directory, inumber, dirent) == -1) {
            fprintf(stderr, "Error: Could not find name for directory '%s'\n", directory);
            free(pathname_copy);
            free(dirent);
            return -1;
        }

        // Update the inode number to the directory entry's inode number
        inumber = dirent->d_inumber;
        // Get the next directory in the pathname
        directory = strtok(NULL, "/");
    }
    free(pathname_copy);   // Clean up allocated memory
    free(dirent);

    // Return the inode number of the last directory
    return inumber;
}
    
