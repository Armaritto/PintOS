#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

void is_valid_address(const void* ptr); //check the validity of the pointer address provided by the user

void exit(int status);

#endif /* userprog/syscall.h */
