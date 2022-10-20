#include "userprog/syscall.h"
#include <syscall-nr.h>
#include "userprog/process.h"
#include "lib/stdio.h"
#include "threads/interrupt.h"
#include "threads/thread.h"


static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  
  printf ("system call!\n");
  int call = f->esp;
  switch (call) {
    case SYS_EXIT:
      process_exit();
      break;
    case SYS_READ:
       int fd = f->frame_pointer;
       void* buffer = f->esp + 2;
       read (fd, buffer, SYS_FILESIZE);
      break;
    case SYS_WRITE:
       int fds = f->esp + 1;
       void* buffer2 = f->esp + 2;
       write (fds, buffer2, SYS_FILESIZE);
      break;
    default:
      break;
  }
    
  thread_exit ();
}
