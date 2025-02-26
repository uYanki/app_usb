/*
 * Copyright (c) 2000-2007 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*-
 * Copyright (c) 1983, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)fcntl.h	8.3 (Berkeley) 1/21/94
 */

/*
 * This file includes the definitions for open and fcntl
 * described by POSIX for <fcntl.h>; it also includes
 * related kernel definitions.
 */
#include <stdio.h>
/* We should not be exporting size_t here.  Temporary for gcc bootstrapping. */

/*
 * File status flags: these are used by open(2), fcntl(2).
 * They are also used (indirectly) in the kernel file structure f_flags,
 * which is a superset of the open/fcntl flags.  Open flags and f_flags
 * are inter-convertible using OFLAGS(fflags) and FFLAGS(oflags).
 * Open/fcntl flags begin with O_; kernel-internal flags begin with F.
 */
/* open-only flags */
#define O_RDONLY  0x0000 /* open for reading only */
#define O_WRONLY  0x0001 /* open for writing only */
#define O_RDWR    0x0002 /* open for reading and writing */
#define O_ACCMODE 0x0003 /* mask for above modes */

/*
 * Kernel encoding of open mode; separate read and write bits that are
 * independently testable: 1 greater than the above.
 *
 * XXX
 * FREAD and FWRITE are excluded from the #ifdef KERNEL so that TIOCFLUSH,
 * which was documented to use FREAD/FWRITE, continues to work.
 */
#define FREAD       0x0001
#define FWRITE      0x0002

#define O_NONBLOCK  0x0004 /* no delay */
#define O_APPEND    0x0008 /* set append mode */
#define O_SYNC      0x0080 /* synch I/O file integrity*/

#define O_SHLOCK    0x0010 /* open with shared file lock */
#define O_EXLOCK    0x0020 /* open with exclusive file lock */
#define O_ASYNC     0x0040 /* signal pgrp when data ready */
#define O_FSYNC     O_SYNC /* source compatibility: do not use */
#define O_NOFOLLOW  0x0100 /* don't follow symlinks */

#define O_CREAT     0x0200 /* create if nonexistant */
#define O_TRUNC     0x0400 /* truncate to zero length */
#define O_EXCL      0x0800 /* error if already exists */

#define FMARK       0x1000 /* mark during gc() */
#define FDEFER      0x2000 /* defer for next gc pass */
#define FHASLOCK    0x4000 /* descriptor holds advisory lock*/
#define O_EVTONLY   0x8000 /* descriptor requested for event notifications only */

#define FWASWRITTEN 0x10000 /* descriptor was written */

#define O_NOCTTY    0x20000 /* don't assign controlling terminal */

#define FNOCACHE    0x40000 /* fcntl(F_NOCACHE, 1) */
#define FNORDAHEAD  0x80000 /* fcntl(F_RDAHEAD, 0) */

#define O_DIRECTORY 0x100000
#define O_SYMLINK   0x200000 /* allow open of a symlink*/
#define O_DSYNC     0x400000 /* synch I/O data integrity */

/* convert from open() flags to/from fflags; convert O_RD/WR to FREAD/FWRITE */
#define FFLAGS(oflags) ((oflags) + 1)
#define OFLAGS(fflags) ((fflags) - 1)

/* bits to save after open */
#define FMASK (FREAD | FWRITE | FAPPEND | FASYNC | FFSYNC | FFDSYNC | FNONBLOCK)
/* bits settable by fcntl(F_SETFL, ...) */
#define FCNTLFLAGS (FAPPEND | FASYNC | FFSYNC | FFDSYNC | FNONBLOCK)

/*
 * The O_* flags used to have only F* names, which were used in the kernel
 * and by fcntl.  We retain the F* names for the kernel f_flags field
 * and for backward compatibility for fcntl.
 */

#define FAPPEND   O_APPEND   /* kernel/compat */
#define FASYNC    O_ASYNC    /* kernel/compat */
#define FFSYNC    O_FSYNC    /* kernel */
#define FFDSYNC   O_DSYNC    /* kernel */
#define FNONBLOCK O_NONBLOCK /* kernel */
#define FNDELAY   O_NONBLOCK /* compat */
#define O_NDELAY  O_NONBLOCK /* compat */

/*
 * Flags used for copyfile(2)
 */

#define CPF_OVERWRITE   1
#define CPF_IGNORE_MODE 2
#define CPF_MASK        (CPF_OVERWRITE | CPF_IGNORE_MODE)

/*
 * Constants used for fcntl(2)
 */

/* command values */
#define F_DUPFD          0  /* duplicate file descriptor */
#define F_GETFD          1  /* get file descriptor flags */
#define F_SETFD          2  /* set file descriptor flags */
#define F_GETFL          3  /* get file status flags */
#define F_SETFL          4  /* set file status flags */
#define F_GETOWN         5  /* get SIGIO/SIGURG proc/pgrp */
#define F_SETOWN         6  /* set SIGIO/SIGURG proc/pgrp */
#define F_GETLK          7  /* get record locking information */
#define F_SETLK          8  /* set record locking information */
#define F_SETLKW         9  /* F_SETLK; wait if blocked */
#define F_CHKCLEAN       41 /* Used for regression test */
#define F_PREALLOCATE    42 /* Preallocate storage */
#define F_SETSIZE        43 /* Truncate a file without zeroing space */
#define F_RDADVISE       44 /* Issue an advisory read async with no copy to user */
#define F_RDAHEAD        45 /* turn read ahead off/on for this fd */
#define F_READBOOTSTRAP  46 /* Read bootstrap from disk */
#define F_WRITEBOOTSTRAP 47 /* Write bootstrap on disk */
#define F_NOCACHE        48 /* turn data caching off/on for this fd */
#define F_LOG2PHYS       49 /* file offset to device offset */
#define F_GETPATH        50 /* return the full path of the fd */
#define F_FULLFSYNC      51 /* fsync + ask the drive to flush to the media */
#define F_PATHPKG_CHECK  52 /* find which component (if any) is a package */
#define F_FREEZE_FS      53 /* "freeze" all fs operations */
#define F_THAW_FS        54 /* "thaw" all fs operations */
#define F_GLOBAL_NOCACHE 55 /* turn data caching off/on (globally) for this file */
#define F_OPENFROM       56 /* SPI: open a file relative to fd (must be a dir) */
#define F_UNLINKFROM     57 /* SPI: open a file relative to fd (must be a dir) */
#define F_CHECK_OPENEVT  58 /* SPI: if a process is marked OPENEVT, or in O_EVTONLY on opens of this vnode */

#define F_ADDSIGS        59 /* add detached signatures */

#define F_MARKDEPENDENCY 60 /* this process hosts the device supporting the fs backing this fd */

#define F_ADDFILESIGS    61 /* add signature from same file (used by dyld for shared libs) */

// FS-specific fcntl()'s numbers begin at 0x00010000 and go up
#define FCNTL_FS_SPECIFIC_BASE 0x00010000

/* file descriptor flags (F_GETFD, F_SETFD) */
#define FD_CLOEXEC 1 /* close-on-exec flag */

/* record locking flags (F_GETLK, F_SETLK, F_SETLKW) */
#define F_RDLCK      1     /* shared or read lock */
#define F_UNLCK      2     /* unlock */
#define F_WRLCK      3     /* exclusive or write lock */
#define F_WAIT       0x010 /* Wait until lock is granted */
#define F_FLOCK      0x020 /* Use flock(2) semantics for lock */
#define F_POSIX      0x040 /* Use POSIX semantics for lock */
#define F_PROV       0x080 /* Non-coelesced provisional lock */
#define F_WAKE1_SAFE 0x100 /* its safe to only wake one waiter */

/*
 * [XSI] The values used for l_whence shall be defined as described
 * in <unistd.h>   */

#define SEEK_SET 0 /* set file offset to offset */
#define SEEK_CUR 1 /* set file offset to current plus offset */
#define SEEK_END 2 /* set file offset to EOF plus offset */

/*
 * [XSI] The symbolic names for file modes for use as values of mode_t
 * shall be defined as described in <sys/stat.h>
 */
/* File type */
#define S_IFMT   0170000 /* [XSI] type of file mask */
#define S_IFIFO  0010000 /* [XSI] named pipe (fifo) */
#define S_IFCHR  0020000 /* [XSI] character special */
#define S_IFDIR  0040000 /* [XSI] directory */
#define S_IFBLK  0060000 /* [XSI] block special */
#define S_IFREG  0100000 /* [XSI] regular */
#define S_IFLNK  0120000 /* [XSI] symbolic link */
#define S_IFSOCK 0140000 /* [XSI] socket */
#define S_IFWHT  0160000 /* whiteout */

/* File mode */
/* Read, write, execute/search by owner */
#define S_IRWXU 0000700 /* [XSI] RWX mask for owner */
#define S_IRUSR 0000400 /* [XSI] R for owner */
#define S_IWUSR 0000200 /* [XSI] W for owner */
#define S_IXUSR 0000100 /* [XSI] X for owner */
/* Read, write, execute/search by group */
#define S_IRWXG 0000070 /* [XSI] RWX mask for group */
#define S_IRGRP 0000040 /* [XSI] R for group */
#define S_IWGRP 0000020 /* [XSI] W for group */
#define S_IXGRP 0000010 /* [XSI] X for group */
/* Read, write, execute/search by others */
#define S_IRWXO  0000007 /* [XSI] RWX mask for other */
#define S_IROTH  0000004 /* [XSI] R for other */
#define S_IWOTH  0000002 /* [XSI] W for other */
#define S_IXOTH  0000001 /* [XSI] X for other */

#define S_ISUID  0004000 /* [XSI] set user id on execution */
#define S_ISGID  0002000 /* [XSI] set group id on execution */
#define S_ISVTX  0001000 /* [XSI] directory restrcted delete */

#define S_ISTXT  S_ISVTX /* sticky bit: not supported */
#define S_IREAD  S_IRUSR /* backward compatability */
#define S_IWRITE S_IWUSR /* backward compatability */
#define S_IEXEC  S_IXUSR /* backward compatability */

/* allocate flags (F_PREALLOCATE) */

#define F_ALLOCATECONTIG 0x00000002 /* allocate contigious space */
#define F_ALLOCATEALL    0x00000004 /* allocate all requested space or no space at all */

/* Position Modes (fst_posmode) for F_PREALLOCATE */

#define F_PEOFPOSMODE 3 /* Make it past all of the SEEK pos modes so that */
                        /* we can keep them in sync should we desire */
#define F_VOLPOSMODE 4  /* specify volume starting postion */

typedef enum {
    FILESEC_OWNER   = 1,
    FILESEC_GROUP   = 2,
    FILESEC_UUID    = 3,
    FILESEC_MODE    = 4,
    FILESEC_ACL     = 5,
    FILESEC_GRPUUID = 6,

    /* XXX these are private to the implementation */
    FILESEC_ACL_RAW       = 100,
    FILESEC_ACL_ALLOCSIZE = 101
} filesec_property_t;
