/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _LIBRPMINSPECT_PARALLEL_H
#define _LIBRPMINSPECT_PARALLEL_H

#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct {
    pid_t    pid;
    int      exit_status;
    /*int    output_fd; - fd is in pfd[] */
    unsigned output_len;
    char     *output;
} parallel_slot_t;

typedef struct {
    unsigned running;
    unsigned max_pids;
    unsigned max_len;
    unsigned ready_fds;
    struct pollfd *pfd;
    parallel_slot_t *slot;
} parallel_t;

extern unsigned default_parallel_processes;

parallel_t *new_parallel(int max);
void delete_parallel(parallel_t *col, int kill_sig);

parallel_slot_t* collect_one(parallel_t *col);
/* unused yet: parallel_slot_t *collect_until_have_free_slot(parallel_t *col); */
void insert_new_pid_and_fd(parallel_t *col, pid_t pid, int fd);

#endif

#ifdef __cplusplus
}
#endif
