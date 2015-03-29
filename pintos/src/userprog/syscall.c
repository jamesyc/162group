#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static uint32_t (*syscall_list[SYS_NULL]) (uint32_t *);
static void syscall_handler (struct intr_frame *);
static uint32_t exit_func (uint32_t *);
static uint32_t null_func (uint32_t *);
static uint32_t write_func (uint32_t *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  syscall_list[SYS_EXIT] = &exit_func;
  syscall_list[SYS_NULL] = &null_func;
  syscall_list[SYS_WRITE] = &write_func;
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  uint32_t* args = ((uint32_t*) f->esp);
  f->eax = syscall_list[args[0]] (args);
}

static uint32_t
exit_func (uint32_t *args) {
  char *name = thread_current()->name;
  char *i = name;
  while (*i != NULL) {
    if (*i == ' ')
      *i = '\0';
    i++;
  }
  printf("%s: exit(%d)\n", name, args[1]);
  thread_exit();
  return args[1];
}

static uint32_t
null_func (uint32_t *args) {
  return args[1] + 1;
}

static uint32_t
write_func (uint32_t *args) {
  printf("%s", (char*) args[2]);
  return args[3];
}