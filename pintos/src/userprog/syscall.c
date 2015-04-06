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
    // syscall_list[SYS_CREATE] = syscall_create;
    // syscall_list[SYS_REMOVE] = syscall_remove;
    // syscall_list[SYS_OPEN] = syscall_open;
    // syscall_list[SYS_FILESIZE] = syscall_filesize;
    // syscall_list[SYS_READ] = syscall_read;
    syscall_list[SYS_WRITE] = syscall_write;
    // syscall_list[SYS_SEEK] = syscall_seek;
    // syscall_list[SYS_TELL] = syscall_tell;
    // syscall_list[SYS_CLOSE] = syscall_close;
    syscall_list[SYS_NULL] = syscall_null;
}

void
syscall_handler (struct intr_frame *f UNUSED) 
{
    uint32_t* args = (uint32_t*) check_ptr((void *) f->esp);
    f->eax = syscall_list[args[0]](args+1);
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
    /* Declare arguments. */
    int fd = args[0];
    const void *buffer = check_ptr((void *) args[1]);
    unsigned length = args[2];
    int checklen = (int) length; //Cast to signed int so can break on < 0


    size_t size = (size_t) length;
    const void *bufptr = buffer;

    if (pg_round_down(buffer) != pg_round_down(buffer + length)) {
        while ((checklen -= PGSIZE) > 0)
            check_ptr(bufptr += PGSIZE);
    }

    size_t write_len = strnlen (buffer, size);
    printf("%.*s", write_len, (char *) buffer);
    
    return write_len;
}

uint32_t
syscall_null (uint32_t *args)
{
    int i = args[0];
    return i + 1;
}


