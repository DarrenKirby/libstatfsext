#include "libstatfsext.h"

int main(void) {
    int n_mounts;
    struct statfs_ext *buf = NULL;
    /* First call: Get the number of mounted filesystems */
    n_mounts = getfsstat_ext(&buf, FS_NUM, 0);
    if (n_mounts == -1) {
		perror("getfsstat_ext: 1st call");
        printf("ERRNO: %i\n", errno);
        exit(EXIT_FAILURE);
    }

    printf("Found %i mounted file systems.\n\n", n_mounts);
    /* Second call: Allocate buffer and get the filesystem data */
    n_mounts = getfsstat_ext(&buf, FS_ALL, 0);
    if (n_mounts == -1) {
		perror("getfsstat_ext: 2nd call");
        printf("ERRNO: %i\n", errno);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n_mounts; i++) {
        printf("Mounted Filesystem: %s\n", buf[i].f_mntfromname);
        printf("\tmnt point: %s\n", buf[i].f_mntonname);
        printf("\tfs type name: %s\n", buf[i].f_fstypename);
        printf("\tfs type: %#x\n", (unsigned int)buf[i].f_type);
        printf("\topt blk size: %i\n", (int)buf[i].f_bsize);
        printf("\tblocks: %d\n", (int)buf[i].f_blocks);
        printf("\tfree blocks: %d\n", (int)buf[i].f_bfree);
        printf("\tfree blocks user: %d\n", (int)buf[i].f_bavail);
        printf("\ttotal nodes: %d\n", (int)buf[i].f_files);
        printf("\tfree nodes: %d\n\n", (int)buf[i].f_ffree);
    }

    free(buf);  // Free the allocated memory
    return 0;
}
