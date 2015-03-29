#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);


/* Syscall structs. */
syscall_fun_t syscall_list[SYS_NULL+1];


void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  syscall_list[SYS_EXIT] = syscall_exit;
  syscall_list[SYS_WRITE] = syscall_write;
  syscall_list[SYS_WAIT] = syscall_wait;
  syscall_list[SYS_NULL] = syscall_null;
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  uint32_t* args = ((uint32_t*) f->esp);
  syscall_list[args[0]](args, &f->eax);
}

void
syscall_exit (uint32_t *args, uint32_t *eax)
{
    char procname[15];

    /* Get the process name without arguments. */
    struct thread *t = thread_current ();
    strlcpy (procname, t->name, 15);

    char *pn_end = strchr(procname, ' ');
    if (pn_end) {
        *pn_end = '\0';
    }

    printf("%s: exit(%d)\n", procname, args[1]);
    
    *eax = args[1];
    thread_exit();
}

void
syscall_wait (uint32_t *args, uint32_t *retval)
{
    tid_t child = args[1];
    *retval = process_wait (child);
}

void
syscall_write (uint32_t *args, uint32_t *eax)
{
    int fd = (int) args[1];
    const char *buffer = (char *) args[2];
    size_t size = (size_t) args[3];

    size_t write_len = strnlen (buffer, size);
    size_t written;

    for (written = 0; written < write_len; written++) {
        printf("%c", *(buffer+written));
    }

    *eax = written;
}

void
syscall_null(uint32_t *args, uint32_t *eax)
{
    *eax = args[1] + 1;
}


