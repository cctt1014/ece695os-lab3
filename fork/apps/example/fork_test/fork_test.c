#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
  int pid;
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int *addr=0x003000;

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Call process fork here
  pid = process_fork();

  // Print valid pid after fork
  if (pid == 0) { // child
    Printf("fork_test (%d): Valid Child Process PTEs after fork:\n", getpid());
    process_print_valid_pte();
  } else { // parent
    Printf("fork_test (%d): Valid Parent Process PTEs after fork:\n", getpid());
    process_print_valid_pte();
  }

  // Parent write to shared memory
  if (pid != 0) {
    Printf("fork_test (%d): Parent write to vaddr 0x003000\n", getpid());
    *addr = 0;
  }

  // Print valid pid after parent write to read only shared memory
  if (pid == 0) { // child
    Printf("fork_test (%d): Valid Child Process PTEs after parent write to read only shared memory:\n", getpid());
    process_print_valid_pte();
  } else { // parent
    Printf("fork_test (%d): Valid Parent Process PTEs after parent write to read only shared memory:\n", getpid());
    process_print_valid_pte();
  }

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("fork_test (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  Printf("fork_test (%d): Done!\n", getpid());
}
