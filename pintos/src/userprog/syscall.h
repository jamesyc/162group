#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdint.h>

typedef int pid_t;

void syscall_init (void);

typedef uint32_t (*syscall_fun_t) (uint32_t *args);

uint32_t syscall_halt (uint32_t *args);
uint32_t syscall_exit (uint32_t *args);
uint32_t syscall_exec (uint32_t *args);
uint32_t syscall_wait (uint32_t *args);
uint32_t syscall_create (uint32_t *args);
uint32_t syscall_remove (uint32_t *args);
uint32_t syscall_open (uint32_t *args);
uint32_t syscall_filesize (uint32_t *args);
uint32_t syscall_read (uint32_t *args);
uint32_t syscall_write (uint32_t *args);
uint32_t syscall_seek (uint32_t *args);
uint32_t syscall_tell (uint32_t *args);
uint32_t syscall_close (uint32_t *args);
uint32_t syscall_null (uint32_t *args);
uint32_t syscall_create (uint32_t *args);
uint32_t syscall_remove (uint32_t *args);

#endif /* userprog/syscall.h */
