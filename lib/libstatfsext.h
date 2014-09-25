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

#define LIBVERSION 1.0.0

#include <stdio.h>
#include <sys/statfs.h>      /* for statfs struct */
#include <linux/limits.h>    /* for PATH_MAX */
#include <errno.h>           /* for perror() */
#include <stdlib.h>          /* for EXIT_FAILURE */
#include <string.h>          /* for strncpy */


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

/* used internally by statfs_ext() and getfsstat_linux() */
int merge_statfs_structs(struct statfs *buf, struct statfs_ext **buf_full) {
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

extern int statfs_ext(const char *path, struct statfs_ext *buf);
extern int getfsstat_linux(struct statfs_ext *buf, long int bufsize);

#endif /* _LIBSTATFSEXT_H */

