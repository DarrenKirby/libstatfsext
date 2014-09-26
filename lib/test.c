#include "libstatfsext.h"

int main(void) {
    int n_mounts = 0;
    struct statfs_ext *buf = malloc(sizeof(struct statfs_ext));
    if (buf == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    
    n_mounts = getfsstat_ext(buf, FS_ALL, 0);
    if (n_mounts == -1) {
        printf("ERRNO: %i\n", errno);
        exit(EXIT_FAILURE);
    }

    printf("Found %i mounted file systems:\n\n", n_mounts);

    int i;
    for (i = 0; i < n_mounts; i++) {
        printf("Mounted Filesystem: %s\n", buf[i].f_mntfromname);
        printf("\tmnt point: %s\n", buf[i].f_mntonname);
        printf("\tfs type name: %s\n", buf[i].f_fstypename);
        printf("\tfs type: %#x\n", (unsigned int)buf[i].f_type);
        printf("\topt blk size: %i\n", (int)buf[i].f_bsize);
        printf("\tblocks: %u\n", (int)buf[i].f_blocks);
        printf("\tfree blocks: %d\n", (int)buf[i].f_bfree);
        printf("\tfree blocks user: %d\n", (int)buf[i].f_bavail);
        printf("\ttotal nodes: %d\n", (int)buf[i].f_files);
        printf("\tfree nodes: %d\n\n", (int)buf[i].f_ffree);

    }

    return 0;
}

