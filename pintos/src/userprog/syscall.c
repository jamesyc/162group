#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "devices/shutdown.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "pagedir.h"

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
  syscall_list[SYS_WAIT] = syscall_wait;
  syscall_list[SYS_WRITE] = syscall_write;
  syscall_list[SYS_NULL] = syscall_null;
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
    uint32_t* args = (uint32_t*) check_ptr((void *) f->esp);
    f->eax = syscall_list[args[0]](args+1);
}


/* Syscall implementations. */

uint32_t
syscall_halt (uint32_t *args UNUSED)
{
    shutdown_power_off ();
}

uint32_t
syscall_exit (uint32_t *args)
{
    /* Print the process name without arguments. */
    struct thread *t = thread_current ();
    char *name_end = strchr(t->name, ' ');

    printf("%.*s: exit(%d)\n", name_end-t->name, t->name, args[0]);
    thread_exit (args[0]);
}

uint32_t
syscall_exec (uint32_t *args)
{
    const char *cmd_line = (char *) check_ptr((void *) args[0]);
    return process_execute (cmd_line);
}

uint32_t
syscall_wait (uint32_t *args)
{
    tid_t child = args[0];
    return process_wait (child);
}

uint32_t
syscall_write (uint32_t *args)
{
    int fd = (int) args[0];
    const char *buffer = (char *) check_ptr((void *) args[1]);
    size_t size = (size_t) args[2];

    size_t write_len = strnlen (buffer, size);
    printf("%.*s", write_len, buffer);
    
    return write_len;
}

uint32_t
syscall_null(uint32_t *args)
{
    return args[0] + 1;
}


