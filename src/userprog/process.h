#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"


/**
 * Executes a new process as specified by FILE_NAME.
 * FILE_NAME is assumed to be a string representing the executable file name.
 * This function creates a new thread to execute the specified process and returns
 * the thread ID of the newly created thread upon success, or TID_ERROR upon failure.
 */
tid_t process_execute(const char *file_name);

/**
 * Waits for a child process identified by the given thread ID to exit and returns its exit status.
 * This function blocks the calling process until the specified child process exits.
 * Upon successful completion, it returns the exit status of the child process.
 */
int process_wait(tid_t);

/**
 * Exits the current process and releases any resources associated with it.
 * This function is typically called when a process completes its execution.
 * It terminates the current thread and trigger cleanup operations, such as releasing
 * allocated memory and closing open files, before exiting the process.
 */
void process_exit(void);

void
process_activate (void);

#endif
/* userprog/process.h */
