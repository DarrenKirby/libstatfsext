/***************************************************************************
 *   libstatfsext.h - implements the BSD* function getfsstat() for Linux   *
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

#ifndef _LIBSTATFSEXT_H
#define _LIBSTATFSEXT_H

#define LIBVERSION 1.1.1

#include <stdio.h>
#include <sys/statfs.h>      /* for statfs struct */
#include <linux/limits.h>    /* for PATH_MAX */
#include <errno.h>           /* for perror() */
#include <stdlib.h>          /* for EXIT_FAILURE */
#include <string.h>          /* for strncpy */
#include <unistd.h>          /* for access() */

#define FS_TYPE_LEN      90
#define MNT_FLAGS_LEN    256

struct mounted_fs_entry {
    char fs_spec[PATH_MAX];           /* device or special file system path */
    char fs_file[PATH_MAX];           /* mount point */
    char fs_vsftype[FS_TYPE_LEN];     /* file system type */
    char fs_mntops[MNT_FLAGS_LEN];    /* mount flags */
    int  fs_freq;                     /* dump */
    int  fs_passno;                   /* pass */
};

#if __WORDSIZE == 32
#define __WORD_TYPE int
#else /* __WORDSIZE == 64 */
#define __WORD_TYPE long int
#endif

struct statfs_ext {
    __WORD_TYPE  f_type;             /* type of filesystem (see below) */
    __WORD_TYPE  f_bsize;            /* optimal transfer block size */
    fsblkcnt_t   f_blocks;           /* total data blocks in filesystem */
    fsblkcnt_t   f_bfree;            /* free blocks in fs */
    fsblkcnt_t   f_bavail;           /* free blocks available to unprivileged user */
    fsfilcnt_t   f_files;            /* total file nodes in filesystem */
    fsfilcnt_t   f_ffree;            /* free file nodes in fs */
    fsid_t       f_fsid;             /* filesystem id */
    __WORD_TYPE  f_namelen;          /* maximum length of filenames */
    __WORD_TYPE  f_frsize;           /* fragment size (since Linux 2.6) */
    __WORD_TYPE  f_spare[5];
    
    /* these extra fields add path info as in the *BSD versions of statfs() */
    char f_fstypename[FS_TYPE_LEN];  /* fs type name */
    char f_mntonname[PATH_MAX];      /* directory on which mounted */
    char f_mntfromname[PATH_MAX];    /* mounted file sytem */
};

#define FS_1 sizeof(struct statfs_ext)
#define FS_2 (FS_1 * 2)
#define FS_3 (FS_1 * 3)
#define FS_4 (FS_1 * 4)
#define FS_5 (FS_1 * 5)
#define FS_6 (FS_1 * 6)
#define FS_7 (FS_1 * 7)
#define FS_8 (FS_1 * 8)
#define FS_9 (FS_1 * 9)
#define FS_10 (FS_1 * 10)
#define FS_11 (FS_1 * 11)
#define FS_12 (FS_1 * 12)
#define FS_13 (FS_1 * 13)
#define FS_14 (FS_1 * 14)
#define FS_15 (FS_1 * 15)
#define FS_16 (FS_1 * 16)
#define FS_17 (FS_1 * 17)
#define FS_18 (FS_1 * 18)
#define FS_19 (FS_1 * 19)
#define FS_20 (FS_1 * 20)
#define FS_ALL 0

/* function prototypes */
extern char *getmntpt(char *path);
extern int statfs_ext(const char *path, struct statfs_ext *struct_buf);
extern int getfsstat_ext(struct statfs_ext *struct_array_buf, long int bufsize, int flags);

#endif /* _LIBSTATFSEXT_H */

