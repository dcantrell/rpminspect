/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <err.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include "rpminspect.h"
#include "parallel.h"

unsigned default_parallel_processes = 0;

static unsigned available_cpus(void)
{
#if 0
    /* This reads /sys/devices/system/cpu/online, which isn't affected by CPU mask */
    return sysconf(_SC_NPROCESSORS_ONLN);
#else
    /* We want "taskset 0x7 rpminspect ..." to correctly assume that only 3 CPUs are available */
    int r;
    uint32_t mask[4096 / 4];

    memset(mask, 0, sizeof(mask));
    r = sched_getaffinity(0, sizeof(mask), (void*)mask);

    if (r != 0) {
        errx(EXIT_FAILURE, "I am not prepared for machines with 4500+ CPUs");
        /* we can write a (re)allocating/looping version when we need to */
    }

    r = 0;

    for (unsigned i = 0; i < sizeof(mask)/sizeof(mask[0]); i++) {
        uint32_t m = mask[i];

        if (m == 0) continue;

        if (~m == 0) { /* fully all-ones word (typical) */
            r += sizeof(mask[0]) * 8;
            continue;
        }

        /* obscure method to count bits in 32-bit word */
        m = m - ((m >> 1) & 0x55555555);
        m = (m & 0x33333333) + ((m >> 2) & 0x33333333);
        r += (((m + (m >> 4)) & 0x0f0f0f0f) * 0x01010101) >> 24;
    }

    return r;
#endif
}

/* If MAX > 0: prepare for up to MAX processes.
 *
 * If MAX is 0, default_parallel_processes is used
 * (which, in turn, is set to sysconf(_SC_NPROCESSORS_ONLN)).
 *
 * If MAX < 0, use default_parallel_processes * (-MAX).
 * For example, if we anticipate that children are simple,
 * fast-finishing processes, it makes sense to spawn 3 * NUM_CPU of them,
 * for system to have something more to do when some of them finish -
 * then use new_parallel(-3).
 */
parallel_t *new_parallel(int max)
{
    parallel_t *col;
    unsigned max_pids;
    unsigned i;

    max_pids = 1;

    if (max < 0 && max > -20 /* new_parallel(-999999) is clearly a bug */) {
        max_pids = -max;
    }

    if (max <= 0) {
        max = default_parallel_processes;

        if (max <= 0) {
            max = available_cpus();

            if ((int)max <= 0) { /* paranoia */
                max = 1;
            }

            if (max > 1024) { /* paranoia */
                max = 1024;
            }

            default_parallel_processes = max;
        }
    }

    max_pids *= max;
    col = xcalloc(1, sizeof(*col));
    col->running = 0;
    col->max_pids = max_pids;
    col->max_len = 64 * 1024 * 1024; /* 64 Mb output sanity limit */
    col->pfd = xcalloc(1, sizeof(*col->pfd) * max_pids);
    col->slot = xcalloc(1, sizeof(*col->slot) * max_pids);

    for (i = 0; i < max_pids; i++) {
        col->pfd[i].fd = -1;
        col->pfd[i].events = POLLIN;
    }

    return col;
}

void delete_parallel(parallel_t *col, int kill_sig)
{
    unsigned i;

    for (i = 0; i < col->max_pids; i++) {
        pid_t pid = col->slot[i].pid;

        if (pid != 0) {
            if (kill_sig) {
                kill(pid, kill_sig);
            } else {
                /* this can be a bug, let user know */
                printf("Note: pid %u is not processed before %s(), waiting for it\n",
                                pid, __func__);
            }

            waitpid(pid, NULL, 0);
        }

        if (col->pfd[i].fd >= 0) {
            close(col->pfd[i].fd);
        }

        free(col->slot[i].output);
    }

    free(col->pfd);
    free(col->slot);
    free(col);
}

parallel_slot_t* collect_one(parallel_t *col)
{
    if (col->running == 0) {
        return NULL;
    }

    for (;;) {
        unsigned i;
        int poll_cnt = col->ready_fds;

        /* Do we already have previous poll() result? */
        if (poll_cnt == 0) {
            /* No. Get new one. IOW: can't avoid doing poll() */
            for (;;) {
                poll_cnt = poll(col->pfd, col->max_pids, -1);

                if (poll_cnt < 0) {
                    if (errno == EINTR) {
                        continue;
                    }
                    err(EXIT_FAILURE, "poll"); /* not supposed to happen */
                }

                if (poll_cnt == 0) { /* timeout??? we didn't ask for one! */
                    err(EXIT_FAILURE, "poll");
                }

                col->ready_fds = poll_cnt;
                break;
            }
        }

        for (i = 0; i < col->max_pids && poll_cnt != 0; i++) {
            char buf[16 * 1024];
            parallel_slot_t *slot = &col->slot[i];

            if (col->pfd[i].revents == 0) {
                    continue;
            }

            /* this fd has data to read */
            int r = read(col->pfd[i].fd, buf, sizeof(buf));
#if 0
            warnx("pfd[%u].revents:0x%x poll_cnt:%u max_pids:%u running:%u read:%d",
                    i, col->pfd[i].revents, poll_cnt, col->max_pids, col->running, r);
#endif
            col->pfd[i].revents = 0; /* avoid checking it later */
            col->ready_fds = --poll_cnt;

            if (r > 0) {
                unsigned newsz = slot->output_len + r;

                if (newsz > col->max_len) { /* usually just paranoia check */
                    errx(EXIT_FAILURE, "maximum length of output exceeded: %u", newsz);
                }

                slot->output = xrealloc(slot->output, newsz + 1);
                char *end = mempcpy(slot->output + slot->output_len, buf, r);
                *end = '\0';
                slot->output_len = newsz;
                continue;
            }

            /* r <= 0: eof/error */
            close(col->pfd[i].fd);
            col->pfd[i].fd = -1;
            /* Wait for the process, get exit status */
            slot->exit_status = 0;

            if (waitpid(slot->pid, &slot->exit_status, 0) < 0) {
                err(EXIT_FAILURE, "waitpid(%u)", slot->pid); /* should not happen */
            }

            col->running--;
            slot->pid = 0;

            /* return this slot */
#if 0
            warnx("returning [%u]: output:%u '%s'", i, slot->output_len, slot->output);
#endif
            return slot;
        }

        /* We are here if we read some data, but no EOFs were seen. */
        /* Return to poll() and wait for more data. */
        col->ready_fds = 0;
    }
}

#if 0 /* unused yet */
parallel_slot_t *collect_until_have_free_slot(parallel_t *col)
{
    if (col->running < col->max_pids) {
        return NULL;
    }

    return collect_one(col);
}
#endif

void insert_new_pid_and_fd(parallel_t *col, pid_t pid, int fd)
{
    unsigned i;

    for (i = 0; i < col->max_pids; i++) {
        parallel_slot_t *slot = &col->slot[i];

        if (slot->pid == 0) {
            col->running++;
            col->pfd[i].fd = fd;
            slot->pid = pid;
            free(slot->output);
            slot->output = NULL;
            slot->output_len = 0;
            return;
        }
    }

    errx(EXIT_FAILURE, "BUG: no free slots");
}
