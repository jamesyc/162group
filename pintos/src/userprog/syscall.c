#include "userprog/syscall.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "devices/shutdown.h"
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);


/* Syscall structs. */
syscall_fun_t syscall_list[SYS_NULL+1];


void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  syscall_list[SYS_HALT] = syscall_halt;
  syscall_list[SYS_EXIT] = syscall_exit;
  syscall_list[SYS_EXEC] = syscall_exec;
  syscall_list[SYS_WRITE] = syscall_write;
  syscall_list[SYS_NULL] = syscall_null;
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  uint32_t* args = ((uint32_t*) f->esp);
  syscall_list[args[0]](args, &f->eax);
}


/* Syscall implementations. */

void
syscall_halt (uint32_t *args UNUSED, uint32_t *retval UNUSED)
{
    shutdown_power_off ();
}

void
syscall_exit (uint32_t *args, uint32_t *retval)
{
    /* Print the process name without arguments. */
    struct thread *t = thread_current ();
    char *name_end = strchr(t->name, ' ');

    printf("%.*s: exit(%d)\n", name_end-t->name, t->name, args[1]);
    
    *retval = args[1];
    thread_exit();
}

void
syscall_exec (uint32_t *args, uint32_t *retval)
{
    const char *cmd_line = (char *) args[1];
}

void
syscall_write (uint32_t *args, uint32_t *retval)
{
    int fd = (int) args[1];
    const char *buffer = (char *) args[2];
    size_t size = (size_t) args[3];

    size_t write_len = strnlen (buffer, size);
    size_t written;

    for (written = 0; written < write_len; written++) {
        printf("%c", *(buffer+written));
    }

    *retval = written;
}

void
syscall_null(uint32_t *args, uint32_t *retval)
{
    *retval = args[1] + 1;
}


