#include "libstatfsext.h"

int main(void) {
    char p1[PATH_MAX];
    char *path = p1;
    getmntpt("/etc/issue", path);
    printf("Dir of /etc/issue: %s\n", path);
    return 0;
}

