/***************************************************************************
 *   libstatfsext.c - implements the BSD* function getfsstat() for Linux   *
 *                                                                         *
 *   Copyright (C) 2024 by Darren Kirby                                    *
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
int __merge_statfs_structs(struct statfs *buf, struct statfs_ext *buf_full) {
    buf_full->f_type    = buf->f_type;
    buf_full->f_bsize   = buf->f_bsize;
    buf_full->f_blocks  = buf->f_blocks;
    buf_full->f_bfree   = buf->f_bfree;
    buf_full->f_bavail  = buf->f_bavail;
    buf_full->f_files   = buf->f_files;
    buf_full->f_ffree   = buf->f_ffree;
    buf_full->f_fsid    = buf->f_fsid;
    buf_full->f_namelen = buf->f_namelen;
    buf_full->f_frsize  = buf->f_frsize;

    for (int i = 0; i < 5; i++) {
        buf_full->f_spare[i] = buf->f_spare[i];
    }

    return SUCCESS;
}


/* used internally by statfs_ext() and getfsstat_ext() */
int __read_proc_mounts(struct mounted_fs_entry *mnt_fs_buf, const char *path) {
    FILE *fp;
    if ((fp = fopen("/proc/mounts", "r")) == NULL) {
        errno = EIO;
        return ERROR;
    }

	memset(mnt_fs_buf, 0, sizeof(struct mounted_fs_entry));

    char line_buf[512];
    char *line_buf_p = line_buf;
    char *tok;

    while (fgets(line_buf_p, 512, fp) != NULL) {
        if (strstr(line_buf_p, "rootfs") != NULL) {
            continue;  // Skip rootfs
        }

        // Tokenize the line
        tok = strtok(line_buf_p, " ");
        if (tok == NULL) continue;
        strncpy(mnt_fs_buf->fs_spec, tok, PATH_MAX - 1);

        tok = strtok(NULL, " ");
        if (tok == NULL) continue;
        strncpy(mnt_fs_buf->fs_file, tok, PATH_MAX - 1);

        tok = strtok(NULL, " ");
        if (tok == NULL) continue;
        strncpy(mnt_fs_buf->fs_vsftype, tok, FS_TYPE_LEN - 1);

        tok = strtok(NULL, " ");
        if (tok == NULL) continue;
        strncpy(mnt_fs_buf->fs_mntops, tok, MNT_FLAGS_LEN - 1);

        // We can use sscanf for the two integers at the end
        tok = strtok(NULL, " ");
        if (tok != NULL) {
            sscanf(tok, "%i", &mnt_fs_buf->fs_freq);
        }

        tok = strtok(NULL, " ");
        if (tok != NULL) {
            sscanf(tok, "%i", &mnt_fs_buf->fs_passno);
        }

        // Check if the mount point matches the desired path
        if (strcmp(mnt_fs_buf->fs_file, path) == 0) {
            fclose(fp);
            return SUCCESS;  // We found the matching entry
        }
    }

    fclose(fp);
    return ERROR;  // No matching mount point found
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

    struct mounted_fs_entry mnt_fs_struct = {};
    struct statfs def_struct_tmp = {};
    
    __read_proc_mounts(&mnt_fs_struct, mount_p);
    
    if (statfs(mount_p, &def_struct_tmp) != 0)
		perror("statfs failed");

    __merge_statfs_structs(&def_struct_tmp, struct_buf);
    
    strncpy(struct_buf->f_fstypename, mnt_fs_struct.fs_vsftype, FS_TYPE_LEN);      
    strncpy(struct_buf->f_mntonname, mnt_fs_struct.fs_file, PATH_MAX);
    strncpy(struct_buf->f_mntfromname, mnt_fs_struct.fs_spec, PATH_MAX);
    
    struct_buf->f_fstypename[FS_TYPE_LEN - 1] = '\0';
	struct_buf->f_mntonname[PATH_MAX - 1] = '\0';
	struct_buf->f_mntfromname[PATH_MAX - 1] = '\0';

    return SUCCESS;
}

int getfsstat_ext(struct statfs_ext **struct_array_buf, long int bufsize, int flags) {
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
    int n_lines = 0;
    while (!feof(fp)) {         /* count lines to determine */
        ch = fgetc(fp);         /* size of struct array     */
        if (ch == '\n') 
            n_lines++;
    }

    if (n_lines <= 0) {
        fclose(fp);
        errno = EIO;
        return ERROR;
    }

    /* If the buffer size is FS_ALL, the user
     * only wants the number of mounts */
    if (bufsize == FS_ALL) {
        fclose(fp);
        return n_lines;
    }

    /* Otherwise, allocate memory for the number 
     * of mounted filesystems */
    *struct_array_buf = malloc(FS_1 * n_lines);
    if (*struct_array_buf == NULL) {
        errno = ENOMEM;
        return ERROR;
    }

    rewind(fp);
    
    struct mounted_fs_entry mounted_fs_struct[n_lines];
    int i = 0;
    char line_buf[256];
    char *line_buf_p = line_buf;
    for (; i < n_lines; i++) {
        line_buf_p = fgets(line_buf_p, sizeof(line_buf), fp);

        int parsed_fields = sscanf(line_buf_p, "%s %s %s %s %d %d\n", 
            mounted_fs_struct[i].fs_spec, mounted_fs_struct[i].fs_file,
            mounted_fs_struct[i].fs_vsftype, mounted_fs_struct[i].fs_mntops,
            &mounted_fs_struct[i].fs_freq, &mounted_fs_struct[i].fs_passno);

        if (parsed_fields < 6) {
            fprintf(stderr, "Error parsing line: %s\n", line_buf_p);
            continue;
        }
    }
    fclose(fp);

    struct statfs_ext tmp_buf = {};

    for (i = 0; i < n_lines; i++) {
		
        if (statfs_ext(mounted_fs_struct[i].fs_file, &tmp_buf) != SUCCESS) {
            /* If statfs_ext fails, set default values */
            tmp_buf.f_type = 0;
            tmp_buf.f_bsize = 0;
            tmp_buf.f_blocks = 0;
            tmp_buf.f_bfree = 0;
            tmp_buf.f_bavail = 0;
            tmp_buf.f_files = 0;
            tmp_buf.f_ffree = 0;
            memset(&tmp_buf.f_fsid, 0, sizeof(tmp_buf.f_fsid));  // Set fsid to 0
            tmp_buf.f_namelen = 0;
            tmp_buf.f_frsize = 0;
            for (int j = 0; j < 5; j++) {
                tmp_buf.f_spare[j] = 0;
            }
        }

        /* Copy the known good values from mounted_fs_struct */
        strncpy(tmp_buf.f_fstypename, mounted_fs_struct[i].fs_vsftype, FS_TYPE_LEN - 1);
        strncpy(tmp_buf.f_mntonname, mounted_fs_struct[i].fs_file, PATH_MAX - 1);
        strncpy(tmp_buf.f_mntfromname, mounted_fs_struct[i].fs_spec, PATH_MAX - 1);

        /* Copy the result from tmp_buf into the array */
        (*struct_array_buf)[i] = tmp_buf;
    }
    
    return n_lines; /* number of mounted filesystems */
}

