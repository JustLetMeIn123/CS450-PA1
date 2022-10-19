#include "userprog/syscall.h"
#include <syscall-nr.h>
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
       set_process_exitcode(thread_current()->tid, call);
       struct process_info *pi = get_process_info(thread_current()->tid) ;
      // printf("%s: exit(%d)\n", pi.name, call);
       thread_exit();
      break;
    case SYS_WAIT:
       if (!check_ptr(f->esp+1)) {
         exit(-1);
         return;
       }

       tid_t pid = f->esp + 1;
       f->eax = process_wait(pid);
      break;
    case SYS_READ:
       if (!check_ptr(f->esp+1) || !check_ptr(f->esp+2) || !check_ptr(f->esp+3)) {
        exit(-1);
       return;
       }

       int fd = f->esp + 1;
       void* buffer = f->esp + 2;
       int len = f->esp + 3;

       if (fd == 0) {
         f->eax = len;
       }
       else {
         //lock_acquire(&fil_lock);
         struct file_info *fi = get_file_info(fd);
         //if (!fi) {
         //lock_release(&fil_lock);
           f->eax = -1;
         //}
       //else {
        int bytes_read = file_read(fi, buffer, len);
        //lock_release(&fil_lock);
        f->eax = bytes_read;
       //}
  }
      break;
    case SYS_WRITE:
       if (!check_ptr(f->esp+1) || !check_ptr(f->esp+2) || !check_ptr(f->esp+3) ) {
         exit(-1);
       return;
       }

       int fds = f->esp + 1;
       void* buffer2 = f->esp + 2;
       int len2 = f->esp + 3;

       if (fds == STDIN_FILENO) {
         exit(-1);
       return;
       }
       else if (fds == STDOUT_FILENO) {
         putbuf(buffer2, len2);
         f->eax = len2;
       }
       else {
       // lock_acquire(&fil_lock);
        struct file_info *fi = get_file_info(fds);
        if (fi == NULL) {
         // lock_release(&fil_lock);
          f->eax = -1;
        }
       else {
        int bytes_written = file_write(fi, buffer2, len2);
       // lock_release(&fil_lock);
        f->eax = bytes_written;
       }
      }
      break;
    default:
      break;
  }
    
  thread_exit ();
}
