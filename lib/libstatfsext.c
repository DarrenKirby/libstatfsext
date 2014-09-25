#include "libstatfsext.h"

int statfs_ext(const char *path, struct statfs_ext *buf) {
    return 0;
}

int getfsstat_linux(struct statfs_ext *buf, long int bufsize) {
    FILE *fp;
    int  n_lines = 0;
    char lines[128];
    char *line = lines;
    
    if ((fp = fopen("/proc/mounts", "r")) == NULL) {
        perror("could not open /proc/mounts");
        exit(EXIT_FAILURE);
    }
    
    char ch;
    while(!feof(fp)) {             /* count lines to determine */
        ch = fgetc(fp);            /* size of struct array     */
        if(ch == '\n') 
            n_lines++;
    }
    rewind(fp);
    
    if (buf == NULL)               /* We have # of mounted fs, might as well bail */
        return n_lines;
    
    struct mounted_fs_entry fse[n_lines];
    int i, i2;

    for (i = 0; i < n_lines; i++) {
        line = fgets(lines, 512, fp);
        sscanf(line, "%s %s %s %s %d %d\n", fse[i].fs_spec, fse[i].fs_file,
                                            fse[i].fs_vsftype, fse[i].fs_mntops,
                                            &fse[i].fs_freq, &fse[i].fs_passno);
    }

    fclose(fp);
    
    /* resize our array of structs */
    buf = realloc(buf, sizeof(struct statfs_ext) * n_lines);
    if (buf == NULL) {
        perror("unable to realloc");
        exit(EXIT_FAILURE);
    }

    struct statfs_ext *f_tmp;
    f_tmp = malloc(sizeof(struct statfs_ext));
    if (f_tmp == NULL) {
        perror("unable to malloc");
        exit(EXIT_FAILURE);
    }
    
    struct statfs *s_tmp;
    s_tmp = malloc(sizeof(struct statfs));
    if (s_tmp == NULL) {
        perror("unable to malloc");
        exit(EXIT_FAILURE);
    }

    for (i2 = 0; i2 < n_lines; i2++) {
        if (statfs(fse[i2].fs_file, s_tmp) != 0) {
            perror("statfs() failed");
            continue; /* might not be fatal */
        }
        merge_statfs_structs(s_tmp, &f_tmp);

        strncpy(f_tmp->f_fstypename, fse[i2].fs_vsftype, FS_TYPE_LEN);      
        strncpy(f_tmp->f_mntonname, fse[i2].fs_file, PATH_MAX);
        strncpy(f_tmp->f_mntfromname, fse[i2].fs_spec, PATH_MAX);

        buf[i2] = *f_tmp;
    }
    
    free(s_tmp);
    free(f_tmp);
    
    return i2;
}
