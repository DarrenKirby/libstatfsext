#include "libstatfsext.h"
int main(void) {

    struct statfs_ext buf_l = {};
    struct statfs_ext *buf = &buf_l;
    statfs_ext("/etc/issue", buf);
    printf("Mounted Filesystem: %s\n", buf->f_mntfromname);
    printf("\tmnt point: %s\n", buf->f_mntonname);
    printf("\tfs type name: %s\n", buf->f_fstypename);
    printf("\tfs type: %#x\n", (unsigned int)buf->f_type);
    printf("\topt blk size: %d\n", (int)buf->f_bsize);
    printf("\tblocks: %u\n", (int)buf->f_blocks);
    printf("\tfree blocks: %d\n", (int)buf->f_bfree);
    printf("\tfree blocks user: %d\n", (int)buf->f_bavail);
    printf("\ttotal nodes: %d\n", (int)buf->f_files);
    printf("\tfree nodes: %d\n\n", (int)buf->f_ffree);

    return 0;
}
