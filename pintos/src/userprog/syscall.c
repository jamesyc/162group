#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "devices/shutdown.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "pagedir.h"

typedef struct {
    syscall_fun_t func;
    int argc;
} func_list;

static void syscall_handler (struct intr_frame *);

/* Syscall structs. */
func_list syscall_list[SYS_NULL+1] = {
    { syscall_halt, 0 },
    { syscall_exit, 1 },
    { syscall_exec, 1 },
    { syscall_wait, 1 },
    { NULL, 0 },//{ syscall_create, 2 },
    { NULL, 0 },//{ syscall_remove, 1 },
    { NULL, 0 },//{ syscall_open, 1 },
    { NULL, 0 },//{ syscall_filesize, 1 },
    { NULL, 0 },//{ syscall_read, 3 },
    { syscall_write, 3 },
    { NULL, 0 },//{ syscall_seek, 2 },
    { NULL, 0 },//{ syscall_tell, 1 },
    { NULL, 0 },//{ syscall_close, 1 },
    { syscall_null, 1 }
};

void
syscall_init (void) 
{
    intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void
syscall_handler (struct intr_frame *f UNUSED) 
{
    uint32_t* args = (uint32_t*) check_ptr((void *) f->esp);

    int i;
    for (i = 0; i < syscall_list[args[0]].argc; i++) {
        check_ptr(&args[i+1]);
    }

    f->eax = syscall_list[args[0]].func (args+1);
}


/* Syscall dispatch functions */

uint32_t
syscall_halt (uint32_t *args UNUSED)
{
    shutdown_power_off ();
}

uint32_t
syscall_exit (uint32_t *args)
{
    int status = args[0];
    thread_exit (status);
}

uint32_t
syscall_exec (uint32_t *args)
{
    const char *file = (char *) args[0];

    const char *cmd_line = (const char *) check_ptr(file);
    return process_execute (cmd_line);
}

uint32_t
syscall_wait (uint32_t *args)
{
    tid_t pid = args[0];
    return process_wait (pid);
}

uint32_t
syscall_write (uint32_t *args)
{
    /* Unwrap arguments. */
    int fd UNUSED = args[0];
    const char *buffer = (const char *) args[1];
    unsigned length = args[2];

    /* Check that all pages are user-readable. */
    const char *bufptr = buffer;
    while (bufptr <= (buffer + length)) {
        check_ptr (bufptr);
        bufptr += PGSIZE;
    }

    size_t write_len = strnlen (buffer, length);
    printf("%.*s", write_len, (char *) buffer);
    
    return write_len;
}

uint32_t
syscall_null (uint32_t *args)
{
    int i = args[0];
    return i + 1;
}


