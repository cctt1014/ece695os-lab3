#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int i;

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Now print a message to show that everything worked
  Printf("process_spawn (%d): Start!\n", getpid());
  for (i = 0; i < 100000; i++) {
    ;
  }
  Printf("process_spawn (%d): End!\n", getpid());

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("process_spawn (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

}
