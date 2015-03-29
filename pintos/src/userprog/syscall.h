#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdint.h>

void syscall_init (void);

typedef void (*syscall_fun_t) (uint32_t *args, uint32_t *retval);

void syscall_halt (uint32_t *args, uint32_t *retval);
void syscall_exit (uint32_t *args, uint32_t *retval);
void syscall_exec (uint32_t *args, uint32_t *retval);
void syscall_write (uint32_t *args, uint32_t *retval);
void syscall_null (uint32_t *args, uint32_t *retval);

#endif /* userprog/syscall.h */
