#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
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

// the only global lock, that we use to avoid race condition of files
static struct lock lock;

// A struct for the file descriptor entry
struct fd_entry
{
    int fd;
    struct file *file;
    struct list_elem elem;
};

static void syscall_handler(struct intr_frame *);
void is_valid_address(const void *ptr);
void halt(void);
void exit(int status);
bool create(char *file, unsigned initial_size);
bool remove(char *file);
tid_t exec(char *cmd_line);
int wait(int pid);
int open(char *file_name);
int fileSize(int fd);
int write(int fd, void *buffer, int size);
int read(int fd, void *buffer, int size);
void seek(int fd, unsigned position);
int tell(int fd);
void close(int fd);
struct opened_file *fd2file(int fd);


void syscall_init(void){
    intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
    lock_init(&lock);
}


/*
 * this function is used to check the validity of the given address
 * it should be inside the user address space via is_user_vaddr.
 * it should also have allocated page (i.e. physical address).
 * it shouldn't be null.
 */
void
is_valid_address(const void *t){
    if (t == NULL || !is_user_vaddr(t) || pagedir_get_page(thread_current()->pagedir, t) == NULL)
        exit(-1);
}

static void
syscall_handler(struct intr_frame *f){
    is_valid_address(f->esp);
    void *esp = f->esp;
    int fd;
    void *buffer;
    int size;
    char *file;
    if(*(int *)esp == SYS_HALT)
        halt();
    else if(*(int *)esp == SYS_EXIT) {
        is_valid_address(esp + 4);
        int status = *((int *)esp + 1);
        exit(status);
    }
    else if(*(int *)esp == SYS_EXEC) {
        is_valid_address(esp + 4);
        char *cmd_line = (char *)(*((int *)esp + 1));
        if (cmd_line == NULL)
            exit(-1);
        lock_acquire(&lock);
        f->eax = exec(cmd_line);
        lock_release(&lock);
    }
    else if(*(int *)esp == SYS_WAIT) {
        is_valid_address(esp + 4);
        int pid = (*((int *)esp + 1));
        f->eax = wait(pid);
    }
    else if(*(int *)esp == SYS_CREATE) {
        is_valid_address(esp + 4);
        is_valid_address(esp + 8);
        file = (char *)(*((uint32_t *)esp + 1));
        unsigned init_size = *((unsigned *)esp + 2);
        if (file == NULL)
            exit(-1);
        f->eax = create(file, init_size);
    }
    else if(*(int *)esp == SYS_REMOVE) {
        is_valid_address(esp + 4);
        file = (char *)(*((uint32_t *)esp + 1));
        if (file == NULL)
            exit(-1);
        f->eax = remove(file);
    }
    else if(*(int *)esp == SYS_OPEN) {
        is_valid_address(esp + 4);
        char *file_name = (char *)(*((uint32_t *)esp + 1));
        if (file_name == NULL)
            exit(-1);
        f->eax = open(file_name);
    }
    else if(*(int *)esp == SYS_FILESIZE) {
        is_valid_address(esp + 4);
        fd = *((uint32_t *)esp + 1);
        f->eax = fileSize(fd);
    }
    else if(*(int *)esp == SYS_READ) {
        is_valid_address(esp + 4);
        is_valid_address(esp + 8);
        is_valid_address(esp + 12);

        fd = *((int *)f->esp + 1);
        buffer = (void *)(*((int *)f->esp + 2));
        size = *((int *)f->esp + 3);
        is_valid_address(buffer + size);
        f->eax = read(fd, buffer, size);
    }
    else if(*(int *)esp == SYS_WRITE) {
        is_valid_address(esp + 4);
        is_valid_address(esp + 8);
        is_valid_address(esp + 12);
        fd = *((uint32_t *)esp + 1);
        buffer = (void *)(*((uint32_t *)esp + 2));
        size = *((unsigned *)esp + 3);
        if (buffer == NULL)
            exit(-1);
        f->eax = write(fd, buffer, size);
    }
    else if(*(int *)esp == SYS_SEEK) {
        is_valid_address(esp + 4);
        is_valid_address(esp + 8);
        fd = *((uint32_t *)esp + 1);
        int pos = (*((unsigned *)esp + 2));
        seek(fd, pos);
    }
    else if(*(int *)esp == SYS_TELL) {
        is_valid_address(esp + 4);
        fd = *((uint32_t *)esp + 1);
        f->eax = tell(fd);
    }
    else if(*(int *)esp == SYS_CLOSE) {
        is_valid_address(esp + 4);
        fd = *((uint32_t *)esp + 1);
        close(fd);
    }
}

/* halt the OS */
void 
halt(void){
    shutdown_power_off();
}

/* Exit and set the parent.child state to your state provided that you have parent */
void 
exit(int status){
    struct thread *current = thread_current()->parent;
    printf("%s: exit(%d)\n", thread_current()->name, status);
    if (current)
        current->childState = status;
    thread_exit();
}


/* create a process and make it execute and return it's TID */
tid_t 
exec(char *cmd_line){
    return process_execute(cmd_line);
}

/* parent waits for the child to finish and return it's status */
int 
wait(int pid){
    return process_wait(pid);
}

/// calls the file system create function to create a file with the given name and size and return true if it's created
bool create(char *file, unsigned initial_size)
{
    bool signal;
    lock_acquire(&lock);
    signal = filesys_create(file, initial_size);
    lock_release(&lock);
    return signal;
}

/// calls the file system remove function to remove a file with the given name and return true if it's removed
bool remove(char *file)
{
    bool signal;
    lock_acquire(&lock);
    signal = filesys_remove(file);
    lock_release(&lock);
    return signal;
}

/// open a file with the given name and return the file descriptor of the opened file
int open(char *file_name)
{
    struct opened_file *open = palloc_get_page(0);
    if (open == NULL){
        palloc_free_page(open);
        return -1;
    }
    lock_acquire(&lock);
    open->ptr = filesys_open(file_name);
    lock_release(&lock);

    // if the pointer is null for any case such as no file name as such or any memory fail
    // then return -1 to indicate that we can't open that file
    if (open->ptr == NULL)
        return -1;
    // increment the directory to get the new index in the file directory table
    open->fd = ++thread_current()->fileDirectory;
    // add to the current open files
    list_push_back(&thread_current()->file_list, &open->elem);
    return open->fd;
}

/// get the size of the file with the given file descriptor
int fileSize(int fd)
{
    struct file *file = fd2file(fd)->ptr;
    if (file == NULL)
        return -1;
    int fileLength;
    lock_acquire(&lock);
    fileLength = file_length(file);
    lock_release(&lock);
    return fileLength;
}

/// change the current position of the file with the given file descriptor to the given position
void seek(int fd, unsigned position)
{
    struct file *file = fd2file(fd)->ptr;
    if (file == NULL)
        return;
    lock_acquire(&lock);
    file_seek(file, position);
    lock_release(&lock);
}

/// get the current position of the file with the given file descriptor
int tell(int fd)
{
    struct file *file = fd2file(fd)->ptr;
    if (file == NULL)
        return -1;
    lock_acquire(&lock);
    int position = file_tell(file);
    lock_release(&lock);
    return position;
}

/// close the file with the given file descriptor and remove it from the opened files list
void close(int fd)
{
    struct opened_file *file = fd2file(fd);
    if (file == NULL)
        return;
    lock_acquire(&lock);
    file_close(file->ptr);
    lock_release(&lock);
    list_remove(&file->elem); //palloc_free_page(file);
}

/// writes (length) bytes from buffer to the open file fd.
int write(int fd, void *buffer, int length)
{
    int sizeActual = 0;
    struct thread *cur = thread_current();

    if (fd == 1)
    {
        // writing to console by the putbuf().
        lock_acquire(&lock);
        putbuf(buffer, length);
        sizeActual = (int)length; // the console is logically infinite so all the buffer is written to stdout
        lock_release(&lock);
    }
        //otherwise we have an opened file to write to, and we alrady have file_write to do that
    else
    {
        // hold the file by the fd
        struct file *f = fd2file(fd)->ptr;
        lock_acquire(&lock);
        if (f == NULL)
            return -1; // no file
        sizeActual = (int)file_write(f, buffer, length);
        lock_release(&lock);
    }
    return sizeActual;
}

/// read from file or from buffer
int read(int fd, void *buffer, int length)
{
    if (fd == 0)
    {//read from keyboard

        for (size_t i = 0; i < length; i++)
        {
            lock_acquire(&lock);// to check no write
            ((char*)buffer)[i] = input_getc();
            lock_release(&lock);
        }
        return length;

    }
    else {
// read from file
        struct thread* t = thread_current();
        struct file* f = fd2file(fd)->ptr;

        if (f == NULL)
        {
            return -1;//no opened file
        }

        int result;//the actual number of bytes be read
        lock_acquire(&lock);
        result = file_read(f,buffer,length);
        lock_release(&lock);
        return result;
    }
}

/// convert from fd to a file object
struct opened_file *fd2file(int fd){
    struct thread *t = thread_current();
    for (struct list_elem *e = list_begin(&t->file_list); e != list_end(&t->file_list);e = list_next(e)){
        struct opened_file *opened = list_entry(e, struct opened_file, elem);
        if (opened->fd == fd)
            return opened;
    }
    return NULL;
}
