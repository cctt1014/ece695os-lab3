#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
  int pid;
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Now print a message to show that everything worked
  // Printf("fork_test (%d): Hello world!\n", getpid());
  pid = process_fork();
  if (pid == 0) { // child
    Printf("Valid Child Process PTEs:\n");
    process_print_valid_pte();
  } else { // parent
    Printf("Valid Parent Process PTEs:\n");
    process_print_valid_pte();
  }

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("fork_test (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  Printf("fork_test (%d): Done!\n", getpid());
}
