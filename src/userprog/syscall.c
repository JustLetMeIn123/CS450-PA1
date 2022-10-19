#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"


static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


struct process_info* get_process_info(tid_t pid) {
  struct list_elem *e;

  struct process_info* pi = NULL;
  lock_acquire(&pil_lock);
  for (e = list_begin (&process_info_list); e != list_end (&process_info_list);
       e = list_next (e))
  {
      struct process_info *p = list_entry (e, struct process_info, elem);
      if (p->pid == pid) {
        pi = p;
        break;
      }
  }

  lock_release(&pil_lock);

  return pi;
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
       printf("%s: exit(%d)\n", pi->name , call);
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

       int fd = f->esp + 1;
       void* buffer = f->esp + 2;
       unsigned int len = f->esp + 3;

       if (fd == STDIN_FILENO) {
         exit(-1);
       return;
       }
       else if (fd == STDOUT_FILENO) {
         putbuf(buffer, len);
         f->eax = len;
       }
       else {
       // lock_acquire(&fil_lock);
        struct file_info *fi = get_file_info(fd);
        if (fi == NULL) {
         // lock_release(&fil_lock);
          f->eax = -1;
        }
       else {
        int bytes_written = file_write(fi, buffer, len);
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
