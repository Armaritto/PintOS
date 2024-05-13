#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrpt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include <string.h>
#include <stdlib.h>
#include "syscall.h"
#include "threads/synch.h"

// Global lock used to prevent race conditions when accessing files.
static struct lock file_lock;

static void syscall_handler(struct intr_frame *);
void validate_address(const void *address);

void halt(void);
void exit(int status);
tid_t exec(char *cmd_line);
int wait(int pid);

bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned size);
int write (int fd, const void *buffer, unsigned size);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);

void syscall_init(void){
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init(&file_lock);
}

// Ensures that the given address is not NULL, within the user address space,
// and maps to a valid page in memory.
void validate_address(const void *address) {
    if (address == NULL || !is_user_vaddr(address) || pagedir_get_page(thread_current()->pagedir, address) == NULL) {
        exit(-1);
    }
}

// Handle system calls
static void syscall_handler(struct intr_frame *f) {
    // Validate the address of the stack pointer
    validate_address(f->esp);

    // Store the stack pointer in a local variable
    void *esp = f->esp;

    // Declare variables for system call parameters
    int fd;
    void *buffer;
    int size;
    char *file;
    
    // Identify the system call and execute the corresponding functionality
    switch (*(int *)esp) {
        case SYS_HALT: // No parameters 
            halt();
            break;

        case SYS_EXIT: // One parameter
            validate_address(esp + 4);
            int status = *((int *)esp + 1);
            exit(status);
            break;

        case SYS_EXEC: // One parameter
            validate_address(esp + 4);
            char *cmd_line = (char *)(*((int *)esp + 1));
            if (cmd_line == NULL)
                exit(-1);
            lock_acquire(&file_lock);
            f->eax = exec(cmd_line);
            lock_release(&file_lock);
            break;

        case SYS_WAIT: // One parameter
            validate_address(esp + 4);
            int pid = (*((int *)esp + 1));
            f->eax = wait(pid);
            break;

        case SYS_CREATE: // Two parameters 
            validate_address(esp + 4);
            validate_address(esp + 8);
            file = (char *)(*((uint32_t *)esp + 1));
            unsigned init_size = *((unsigned *)esp + 2);
            if (file == NULL)
                exit(-1);
            f->eax = create(file, init_size);
            break;

        case SYS_REMOVE: // One parameter
            validate_address(esp + 4);
            file = (char *)(*((uint32_t *)esp + 1));
            if (file == NULL)
                exit(-1);
            f->eax = remove(file);
            break;

        case SYS_OPEN: // One parameter
            validate_address(esp + 4);
            char *file_name = (char *)(*((uint32_t *)esp + 1));
            if (file_name == NULL)
                exit(-1);
            f->eax = open(file_name);
            break;

        case SYS_FILESIZE: // One parameter
            validate_address(esp + 4);
            fd = *((uint32_t *)esp + 1);
            f->eax = file_size(fd);
            break;

        case SYS_READ: // Three parameters
            validate_address(esp + 4);
            validate_address(esp + 8);
            validate_address(esp + 12);

            fd = *((int *)f->esp + 1);
            buffer = (void *)(*((int *)f->esp + 2));
            size = *((int *)f->esp + 3);

            validate_address(buffer + size);

            f->eax = read(fd, buffer, size);
            break;

        case SYS_WRITE: // Three parameters
            validate_address(esp + 4);
            validate_address(esp + 8);
            validate_address(esp + 12);
            fd = *((uint32_t *)esp + 1);
            buffer = (void *)(*((uint32_t *)esp + 2));
            size = *((unsigned *)esp + 3);
            if (buffer == NULL)
                exit(-1);

            f->eax = write(fd, buffer, size);
            break;

        case SYS_SEEK: // Two parameters
            validate_address(esp + 4);
            validate_address(esp + 8);
            fd = *((uint32_t *)esp + 1);
            int pos = (*((unsigned *)esp + 2));
            seek(fd, pos);
            break;

        case SYS_TELL: // One parameter
            validate_address(esp + 4);
            fd = *((uint32_t *)esp + 1);
            f->eax = tell(fd);
            break;

        case SYS_CLOSE: // One parameter
            validate_address(esp + 4);
            fd = *((uint32_t *)esp + 1);
            close(fd);
            break;

        default:
            break;
    }
}



void halt(void){
  // amin
  shutdown_power_off();
}


void exit(int status){
  // amin
  printf("%s: exit(%d)\n", thread_current()->name, status);
  if (thread_current()->parent){
    thread_current()->parent->childState = status;
  }
  thread_exit();
}


tid_t exec(char *cmd_line){
  // amin
  return process_execute(cmd_line);
}

int wait(int pid){
   // amin
  return process_wait(pid);
}

bool create (const char *file, unsigned initial_size){
// armia
return true;
}

bool remove (const char *file){
// armia
return true;
}

int open (const char *file){
// armia
return 0;
}

int filesize (int fd){
// armia
return 0;
}

int read (int fd, void *buffer, unsigned size){
// armia
return 0;
}

int write (int fd, const void *buffer, unsigned size){
// armia
return 0;
}

void seek (int fd, unsigned position){
// armia
}

unsigned tell (int fd){
// armia
return 0;
}

void close (int fd){
// armia 
}
