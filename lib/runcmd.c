/*
 * Copyright The rpminspect Project Authors
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <err.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>

#include "rpminspect.h"

#define RD STDIN_FILENO
#define WR STDOUT_FILENO

extern char **environ;

/*
 * Generic fork()/execvpe() wrapper to return the output of the
 * process and the exit code (if desired).  This function returns an
 * allocated string of the output from the program that ran or NULL if
 * there was no output.
 *
 * The first argument is a pointer to an int that will hold the exit
 * code from execvpe().  If this pointer is NULL, then the caller does
 * not want the exit code.  Internally the exit code will be used to
 * determine if the process was signaled or not, but the exit code
 * will not be given back to the caller.
 *
 * The second argument is the command followed by any additional
 * arguments that should be included with it.  Note that it is not a
 * format string, all of the subsequent arguments need to be strings
 * because they all get concatenated together.
 */
char *run_cmd_vpe(int *exitcode, const char *workdir, char **argv)
{
    int i = 0;
    int pfd[2];
    int status = 0;
    pid_t proc = 0;
    FILE *reader = 0;
    char *signame = NULL;
    char *output = NULL;
    char *tail = NULL;
    size_t n = BUFSIZ;
    char *buf = NULL;
    char cwd[PATH_MAX + 1];

    assert(argv != NULL);
    assert(argv[0] != NULL);

    /* use working directory if given one */
    if (workdir) {
        /* save current directory */
        memset(cwd, '\0', sizeof(cwd));

        if (getcwd(cwd, PATH_MAX) == NULL) {
            err(RI_PROGRAM_ERROR, "getcwd");
        }

        /* power through if it fails */
        if (chdir(workdir) == -1) {
            warn("chdir");
        }
    }

    /* create pipes to interact with the child */
    if (pipe(pfd) == -1) {
        if (exitcode) {
            *exitcode = EXIT_FAILURE;
        }

        warn("pipe");
        return NULL;
    }

    /* run the command */
    proc = fork();

    if (proc == 0) {
        /* connect the output */
        if (dup2(pfd[WR], STDOUT_FILENO) == -1 || dup2(pfd[WR], STDERR_FILENO) == -1) {
            warn("dup2");
            _exit(EXIT_FAILURE);
        }

        /* close the pipe */
        if (close(pfd[RD]) == -1 || close(pfd[WR]) == -1) {
            warn("dup2");
            _exit(EXIT_FAILURE);
        }

        setlinebuf(stdout);

        /* run the command */
        if (execvpe(argv[0], argv, environ) == -1) {
            warn("execvpe");
            _exit(EXIT_FAILURE);
        }
    } else if (proc == -1) {
        /* failure */
        warn("fork");
    } else {
        /* close the pipe */
        if (close(pfd[WR]) == -1) {
            warn("close");
        }

        /*
         * Read in all of the information back from the command and store
         * it as our result.  Just concatenate the string as we read it
         * back in buffer size chunks.
         */
        reader = fdopen(pfd[RD], "r");

        if (reader == NULL) {
            if (exitcode) {
                *exitcode = EXIT_FAILURE;
            }

            return NULL;
        }

        output = NULL;
        buf = calloc(1, n);
        assert(buf != NULL);

        while (getline(&buf, &n, reader) != -1) {
            xasprintf(&tail, "%s%s", (output == NULL) ? "" : output, buf);
            assert(tail != NULL);
            free(output);
            output = tail;
        }

        free(buf);

        if (fclose(reader) == -1) {
            warn("fclose");
        }

        /* wait for the command */
        if (waitpid(proc, &status, 0) == -1) {
            if (exitcode) {
                *exitcode = EXIT_FAILURE;
            }

            warn("waitpid");
        }

        if (WIFEXITED(status)) {
            if (exitcode) {
                *exitcode = WEXITSTATUS(status);
            }
        } else if (WIFSIGNALED(status)) {
            if (exitcode) {
                *exitcode = EXIT_FAILURE;
            }

            /* generate a string with the signal name if possible */
            i = WTERMSIG(status);

            if (strsignal(i) == NULL) {
                xasprintf(&signame, _("%d"), i);
            } else {
                xasprintf(&signame, _("%d (%s)"), i, strsignal(i));
            }

            /* generic output indicating the command we tried to run and the signal received */
            if (output) {
                xasprintf(&tail, _("%s\n\n%s tried to run the command and it received signal %s"), output, COMMAND_NAME, signame);
                free(output);
                output = tail;
            } else {
                xasprintf(&output, _("%s tried to run the command and it received signal %s"), COMMAND_NAME, signame);
            }

            free(signame);
        }

        /* There may be no results from the tool */
        if (output != NULL) {
            /* Trim trailing newline */
            tail = rindex(output, '\n');

            if (tail != NULL) {
                tail[strcspn(tail, "\n")] = 0;
            }
        }
    }

    /* go back to where we started */
    if (workdir && chdir(cwd) == -1) {
        warn("chdir");
    }

    return output;
}

/*
 * Wrapper for run_cmd_vpe() that lets you pass in varargs instead of a
 * string_list_t.
 */
char *run_cmd(int *exitcode, const char *workdir, const char *cmd, ...)
{
    va_list ap;
    char *output = NULL;
    char *element = NULL;
    char **argv = NULL;
    int i = 0;

    assert(cmd != NULL);

    /* convert varargs to a string array */
    argv = calloc(2, sizeof(*argv));
    assert(argv != NULL);

    /* add the command */
    argv[i] = strdup(cmd);
    assert(argv[i] != NULL);
    i++;
    argv[i] = NULL;

    /* Add the remaining elements */
    va_start(ap, cmd);

    while ((element = va_arg(ap, char *)) != NULL) {
        i++;
        argv = realloc(argv, sizeof(*argv) * (i + 1));
        assert(argv != NULL);

        argv[i - 1] = strdup(element);
        assert(argv[i - 1] != NULL);
        argv[i] = NULL;
    }

    va_end(ap);

    /* run the command */
    output = run_cmd_vpe(exitcode, workdir, argv);

    /* clean up */
    free_argv(argv);

    return output;
}

/*
 * Split a string in to a char ** of all the arguments, terminated
 * with a NULL entry.  Caller must free.
 */
char **build_argv(const char *cmd)
{
    char **r = NULL;
    int i = 0;
    string_list_t *list = NULL;
    string_entry_t *entry = NULL;

    if (cmd == NULL) {
        return NULL;
    }

    list = strsplit(cmd, " \t");
    assert(list != NULL);

    TAILQ_FOREACH(entry, list, items) {
        r = realloc(r, sizeof(*r) * (i + 2));
        assert(r != NULL);

        r[i] = strdup(entry->data);
        assert(r[i] != NULL);
        r[i + 1] = NULL;
        i++;
    }

    list_free(list, free);
    return r;
}

/*
 * Clean up a char **
 */
void free_argv(char **argv)
{
    int i = 0;

    if (argv == NULL) {
        return;
    }

    for (i = 0; argv[i] != NULL; i++) {
        free(argv[i]);
    }

    free(argv);
    return;
}

/*
 * Free one of the command line option tables.
 */
void free_argv_table(struct rpminspect *ri, string_list_map_t *table)
{
    string_list_map_t *hentry = NULL;
    string_list_map_t *tmp_hentry = NULL;

    assert(ri != NULL);
    assert(ri->arches != NULL);

    if (table == NULL) {
        return;
    }

    HASH_ITER(hh, table, hentry, tmp_hentry) {
        HASH_DEL(table, hentry);
        free(hentry->key);
        list_free(hentry->value, free);
    }

    return;
}
