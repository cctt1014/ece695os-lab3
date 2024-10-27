#include "usertraps.h"
#include "misc.h"


void recursive_func(int i) {
  int arr_1KB[256];
  Printf("grow_stack (%d): recursive i=%d\n", getpid(), i);

  if (i == 0) {
    return;
  } else {
    recursive_func(i-1);
  }
  return;
}


void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done

  Printf("grow_stack (%d): Start!\n", getpid());

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Using recursive function to increase user function call stack
  recursive_func(10);

 
  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("grow_stack (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  Printf("grow_stack (%d): Done!\n", getpid());
}
