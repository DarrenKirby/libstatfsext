/***************************************************************************
 *   libstatfsext.c - implements the BSD* function getfsstat() for Linux   *
 *                                                                         *
 *   Copyright (C) 2014 by Darren Kirby                                    *
 *   bulliver@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "libstatfsext.h"

/* used internally by statfs_ext() and getfsstat_ext() */
int __merge_statfs_structs(struct statfs *buf, struct statfs_ext **buf_full) {
    int i;
    (*buf_full)->f_type    = buf->f_type;
    (*buf_full)->f_bsize   = buf->f_bsize;
    (*buf_full)->f_blocks  = buf->f_blocks;
    (*buf_full)->f_bfree   = buf->f_bfree;
    (*buf_full)->f_bavail  = buf->f_bavail;
    (*buf_full)->f_files   = buf->f_files;
    (*buf_full)->f_ffree   = buf->f_ffree;
    (*buf_full)->f_fsid    = buf->f_fsid;
    (*buf_full)->f_namelen = buf->f_namelen;
    (*buf_full)->f_frsize  = buf->f_frsize;
    for (i = 0; i < 5; i++)
        (*buf_full)->f_spare[i]   = buf->f_spare[i];
    return 0;
}

/* used internally by statfs_ext() and getfsstat_ext() */
int __read_proc_mounts(struct mounted_fs_entry *bf, char *path) {
    FILE *fp;
    char lines[256];
    char *line = lines;
    char *tok;
    char line_tmp[256];
    char *line_tmp_p = line_tmp;
    
    if ((fp = fopen("/proc/mounts", "r")) == NULL) {
        errno = EIO;
        return -1;
    }

    while (fgets (line, 256, fp) != NULL) {
        if (strstr(line, "rootfs") != NULL)
            continue;
        strcpy(line_tmp_p, line);
        tok = strtok(line_tmp_p, " ");
        tok = strtok(NULL, " ");
        if (strcmp(tok, path) == 0) {
            sscanf(line, "%s %s %s %s %i %i\n", bf->fs_spec, bf->fs_file,
                                        bf->fs_vsftype, bf->fs_mntops,
                                        &bf->fs_freq, &bf->fs_passno);
        } else {
            continue;
        }
    }
    fclose(fp);
    return 1; 
}

int statfs_ext(const char *path, struct statfs_ext *buf) {
    /* check size of path arg */
    if (strlen(path) > PATH_MAX) {
        errno = ENAMETOOLONG;
        return -1;
    }
    
    /* check if path exists */
    if (access(path, F_OK) != 0) {
        errno = ENOENT;
        return -1;
    }
    
    /* check if we have access to path */
    if (access(path, R_OK) != 0) {
        errno = EACCES;
        return -1;
    }
    
    /* this is a crufty dirty hack to find the mountpoint of 'path'
     * it will be replaced by a pure-C solution (a la 'df') soon enough */
    char command[PATH_MAX];
    char *command_p = command;
    char mount_path[PATH_MAX];
    char *mount_p = mount_path;
    char type[] = "r\0";
    char *type_p = type;
    FILE *pp;
    sprintf(command_p, "df %s | awk 'NR==1 {next} {print $6; exit}'", path);
    pp = popen(command_p, type_p);
    mount_p = strtok(fgets(mount_p, PATH_MAX, pp), "\n");
    pclose(pp);
    /* end crufty dirty hack */ 
    
    struct mounted_fs_entry *mfe = malloc(sizeof(struct mounted_fs_entry));
    if (buf == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    struct statfs *tmp = malloc(sizeof(struct statfs));
    if (buf == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    __read_proc_mounts(mfe, mount_p);
    statfs(mount_p, tmp);
    __merge_statfs_structs(tmp, &buf);
    
    strncpy(buf->f_fstypename, mfe->fs_vsftype, FS_TYPE_LEN);      
    strncpy(buf->f_mntonname, mfe->fs_file, PATH_MAX);
    strncpy(buf->f_mntfromname, mfe->fs_spec, PATH_MAX);
   
    free(mfe);
    free(tmp);
    return 0;
}

int getfsstat_ext(struct statfs_ext *buf, long int bufsize, int flags) {
    /* make sure the bufsize is reasonable */
    if (bufsize < sizeof(struct statfs_ext)) {
        errno = EINVAL;
        return -1;
    }
    
    FILE *fp;
    int  n_lines = 0;
    char lines[256];
    char *line = lines;
    
    if ((fp = fopen("/proc/mounts", "r")) == NULL) {
        errno = EIO;
        return -1;
    }
    char ch;
    while(!feof(fp)) {             /* count lines to determine */
        ch = fgetc(fp);            /* size of struct array     */
        if(ch == '\n') 
            n_lines++;
    }
    rewind(fp);
    
    struct mounted_fs_entry bf[n_lines];
    /* loop over lines and fill the struct */
    int i = 0;
    for (; i < n_lines; i++) {
        line = fgets(line, 512, fp);
        sscanf(line, "%s %s %s %s %d %d\n", bf[i].fs_spec, bf[i].fs_file,
                                            bf[i].fs_vsftype, bf[i].fs_mntops,
                                            &bf[i].fs_freq, &bf[i].fs_passno);
    }
    fclose(fp);
    
    if (buf == NULL)           /* We have # of mounted fs, might as well bail */ 
        return n_lines;

    /* resize our array of structs */
    buf = realloc(buf, sizeof(struct statfs_ext) * n_lines);
    if (buf == NULL) {
        errno = ENOMEM;
        return -1;
    }

    struct statfs_ext *f_tmp;
    f_tmp = malloc(sizeof(struct statfs_ext));
    if (f_tmp == NULL) {
        errno = ENOMEM;
        return -1;
    }
    
    struct statfs *s_tmp;
    s_tmp = malloc(sizeof(struct statfs));
    if (s_tmp == NULL) {
        errno = ENOMEM;
        return -1;
    }

    int ii = 0;
    for (; ii < n_lines; ii++) {

        if (statfs(bf[ii].fs_file, s_tmp) != 0) {
            if (n_lines == 1) {
                errno = EIO;
                return -1;
            }
            continue; /* might not be fatal */
        }
        __merge_statfs_structs(s_tmp, &f_tmp);

        strncpy(f_tmp->f_fstypename, bf[ii].fs_vsftype, FS_TYPE_LEN);      
        strncpy(f_tmp->f_mntonname, bf[ii].fs_file, PATH_MAX);
        strncpy(f_tmp->f_mntfromname, bf[ii].fs_spec, PATH_MAX);

        buf[ii] = *f_tmp;
    }
    
    free(s_tmp);
    free(f_tmp);
    
    return i; /* number of mounted filesystems found */
}
