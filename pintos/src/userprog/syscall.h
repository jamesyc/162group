#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdint.h>

void syscall_init (void);

typedef void (*syscall_fun_t) (uint32_t *args, uint32_t *eax);

void syscall_exit (uint32_t *args, uint32_t *eax);
void syscall_write (uint32_t *args, uint32_t *eax);
void syscall_wait (uint32_t *args, uint32_t *eax);
void syscall_null (uint32_t *args, uint32_t *eax);

#endif /* userprog/syscall.h */
