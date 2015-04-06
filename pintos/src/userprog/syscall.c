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

typedef int pid_t;

static void syscall_handler (struct intr_frame *);
void halt (void);
void exit (int status);
pid_t exec (const char *file);
int wait (pid_t);
/*bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned length);*/
int write (int fd, const void *buffer, unsigned length);
/*void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);*/
int null (int i);

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
    halt();
}

uint32_t
syscall_exit (uint32_t *args)
{
    exit(args[0]);
}

uint32_t
syscall_exec (uint32_t *args)
{
    return exec((char *) args[0]);
}

uint32_t
syscall_wait (uint32_t *args)
{
    return wait((pid_t) args[0]);
}

uint32_t
syscall_write (uint32_t *args)
{
    return write((int) args[0], (void *) args[1], (size_t) args[2]);
}

uint32_t
syscall_null (uint32_t *args)
{
    return null(args[0]);
}

/* Syscall implementations */

void
halt (void)
{
    shutdown_power_off ();
}

void
exit (int status)
{
    /* Print the process name without arguments. */
    struct thread *t = thread_current ();
    char *name_end = strchr(t->name, ' ');

    printf("%.*s: exit(%d)\n", name_end-t->name, t->name, status);
    thread_exit (status);
}

pid_t
exec (const char *file)
{
    const char *cmd_line = (const char *) check_ptr(file);
    return process_execute (cmd_line);
}

int
wait (pid_t pid)
{
    return process_wait ((tid_t) pid); /*pid of child*/
}

/*bool
create (const char *file, unsigned initial_size)
{
    return NULL;
}

bool
remove (const char *file)
{
    return NULL;
}

int
open (const char *file)
{
    return NULL;
}

int
filesize (int fd)
{
    return NULL;
}

int
read (int fd, void *buffer, unsigned length)
{
    return NULL;
}*/

int
write (int fd, const void *buffer, unsigned length)
{
    size_t size = (size_t) length;
    const void *bufptr = buffer;

    if (pg_round_down(buffer) != pg_round_down(buffer + length)) {
        while (length--)
            check_ptr(bufptr++);
    }

    size_t write_len = strnlen (buffer, size);
    printf("%.*s", write_len, (char *) buffer);
    
    return write_len;
}

/*void
seek (int fd, unsigned position)
{
    
}

unsigned
tell (int fd)
{
    return NULL;
}

void
close (int fd)
{
    
}*/

int
null (int i)
{
    return i + 1;
}