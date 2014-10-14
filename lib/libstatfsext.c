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
    int i;
    for (i = 0; i < 5; i++)
        (*buf_full)->f_spare[i]   = buf->f_spare[i];
    return SUCCESS;
}

/* used internally by statfs_ext() and getfsstat_ext() */
int __read_proc_mounts(struct mounted_fs_entry *mnt_fs_buf, char *path) {
    FILE *fp;
    if ((fp = fopen("/proc/mounts", "r")) == NULL) {
        errno = EIO;
        return ERROR;
    }

    char line_buf[256];
    char *line_buf_p = line_buf;
    char *tok;
    char line_tmp[256];
    char *line_tmp_p = line_tmp;   
    while (fgets (line_buf_p, 256, fp) != NULL) {
        if (strstr(line_buf_p, "rootfs") != NULL)
            continue;
        strncpy(line_tmp_p, line_buf_p, 256);     /* strtok() mutates the string... */
        tok = strtok(line_tmp_p, " ");
        tok = strtok(NULL, " ");
        if (strcmp(tok, path) == 0) {
            sscanf(line_buf_p, "%s %s %s %s %i %i\n", mnt_fs_buf->fs_spec, mnt_fs_buf->fs_file,
                                        mnt_fs_buf->fs_vsftype, mnt_fs_buf->fs_mntops,
                                        &mnt_fs_buf->fs_freq, &mnt_fs_buf->fs_passno);
        } else {
            continue;
        }
    }
    fclose(fp);
    return SUCCESS; 
}

int getmntpt(const char *path, char *mount_point) {
    struct stat cur_stat;
    struct stat last_stat;
    
    char dir_name[PATH_MAX];
    char *dirname_p = dir_name;
    char cur_cwd[255];
    char *cur_cwd_p = cur_cwd;
    char saved_cwd[PATH_MAX];
    if (getcwd(saved_cwd, PATH_MAX) == NULL) {
        errno = EIO;
        return ERROR;
    }
    
    if (lstat(path, &cur_stat) < 0) {
        errno = EIO;
        return ERROR;
    }
    
    if (S_ISDIR (cur_stat.st_mode)) {
        last_stat = cur_stat;
        if (chdir("..") < 0)
            return ERROR;
        if (getcwd(cur_cwd_p, 255) == NULL) {
            errno = EIO;
            return ERROR;
        }
    } else { /* path is a file */
        size_t path_len, suffix_len, dir_len;
        path_len = strlen(path);
        suffix_len = strlen(strrchr(path, 47)); /* 47 = '/' */
        dir_len = path_len - suffix_len;
        dirname_p = strncpy(dirname_p, path, dir_len);
        if (chdir(dirname_p) < 0) 
            return ERROR;
        if (lstat(".", &last_stat) < 0)
            return ERROR;
    }
    
    for (;;) {
        if (lstat("..", &cur_stat) < 0)
            return ERROR;
        if (cur_stat.st_dev != last_stat.st_dev || cur_stat.st_ino == last_stat.st_ino)
            break; /* this is the mount point */
        if (chdir("..") < 0)
            return ERROR;
        last_stat = cur_stat;
    }

    if (getcwd(mount_point, PATH_MAX) == NULL)
        return ERROR;
    if (chdir(saved_cwd) < 0)
        return ERROR;
    return SUCCESS;
}

int statfs_ext(const char *path, struct statfs_ext *struct_buf) {
    /* check size of path arg */
    if (strlen(path) > PATH_MAX) {
        errno = ENAMETOOLONG;
        return ERROR;
    }
    
    /* check if path exists */
    if (access(path, F_OK) != 0) {
        errno = ENOENT;
        return ERROR;
    }
    
    /* check if we have access to path */
    if (access(path, R_OK) != 0) {
        errno = EACCES;
        return ERROR;
    }

    char mount_path[PATH_MAX];
    char *mount_p = mount_path;

    if (getmntpt(path, mount_p) != 0)
        return ERROR;

    struct mounted_fs_entry mnt_fs_struct_l = {};
    struct mounted_fs_entry *mnt_fs_struct = &mnt_fs_struct_l;

    struct statfs def_struct_tmp_l = {};
    struct statfs *def_struct_tmp = &def_struct_tmp_l;
    
    __read_proc_mounts(mnt_fs_struct, mount_p);
    statfs(mount_p, def_struct_tmp);
    __merge_statfs_structs(def_struct_tmp, &struct_buf);
    
    strncpy(struct_buf->f_fstypename, mnt_fs_struct->fs_vsftype, FS_TYPE_LEN);      
    strncpy(struct_buf->f_mntonname, mnt_fs_struct->fs_file, PATH_MAX);
    strncpy(struct_buf->f_mntfromname, mnt_fs_struct->fs_spec, PATH_MAX);

    return SUCCESS;
}

int getfsstat_ext(struct statfs_ext *struct_array_buf, long int bufsize, int flags) {
    /* make sure the bufsize is reasonable */
    if ((bufsize < FS_1) && (bufsize != FS_ALL)) {
        errno = EINVAL;
        return ERROR;
    }

    FILE *fp;
    if ((fp = fopen("/proc/mounts", "r")) == NULL) {
        errno = EIO;
        return ERROR;
    }
    
    char ch;
    int  n_lines = 0;
    while(!feof(fp)) {             /* count lines to determine */
        ch = fgetc(fp);            /* size of struct array     */
        if(ch == '\n') 
            n_lines++;
    }
    
    if (struct_array_buf == NULL) {           /* We have # of mounted fs, might as well bail */ 
        fclose(fp);
        return n_lines;
    }
    
    rewind(fp);
    
    struct mounted_fs_entry mounted_fs_struct[n_lines];
    /* loop over lines and fill the struct */
    int i = 0;
    char line_buf[256];
    char *line_buf_p = line_buf;
    for (; i < n_lines; i++) {
        line_buf_p = fgets(line_buf_p, 512, fp);
        sscanf(line_buf_p, "%s %s %s %s %d %d\n", mounted_fs_struct[i].fs_spec, mounted_fs_struct[i].fs_file,
                                            mounted_fs_struct[i].fs_vsftype, mounted_fs_struct[i].fs_mntops,
                                            &mounted_fs_struct[i].fs_freq, &mounted_fs_struct[i].fs_passno);
    }
    fclose(fp);
    
    /* How many structs to return? */
    if (bufsize == FS_ALL)
        ;
    else
        n_lines = bufsize / FS_1;
    
    /* resize our array of structs */
    struct_array_buf = realloc(struct_array_buf, sizeof(struct statfs_ext) * n_lines);
    if (struct_array_buf == NULL) {
        errno = ENOMEM;
        return ERROR;
    }

    struct statfs_ext ext_struct_tmp_l = {};
    struct statfs_ext *ext_struct_tmp = &ext_struct_tmp_l;

    struct statfs def_struct_tmp_l = {};
    struct statfs *def_struct_tmp = &def_struct_tmp_l;
    
    i = 0;
    for (; i < n_lines; i++) {

        if (statfs(mounted_fs_struct[i].fs_file, def_struct_tmp) != 0) {
            if (n_lines == 1) {
                errno = EIO;
                return ERROR;
            }
            continue; /* might not be fatal */
        }
        __merge_statfs_structs(def_struct_tmp, &ext_struct_tmp);

        strncpy(ext_struct_tmp->f_fstypename, mounted_fs_struct[i].fs_vsftype, FS_TYPE_LEN);      
        strncpy(ext_struct_tmp->f_mntonname, mounted_fs_struct[i].fs_file, PATH_MAX);
        strncpy(ext_struct_tmp->f_mntfromname, mounted_fs_struct[i].fs_spec, PATH_MAX);

        struct_array_buf[i] = *ext_struct_tmp;
    }
    
    return n_lines; /* number of mounted filesystems  */
}
