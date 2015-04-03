#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdint.h>

void syscall_init (void);

typedef uint32_t (*syscall_fun_t) (uint32_t *args);

uint32_t syscall_halt (uint32_t *args);
uint32_t syscall_exit (uint32_t *args);
uint32_t syscall_exec (uint32_t *args);
uint32_t syscall_wait (uint32_t *args);
uint32_t syscall_write (uint32_t *args);
uint32_t syscall_null (uint32_t *args);

#endif /* userprog/syscall.h */
