#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD "hello_world.dlx.obj"
#define BEYOND_MAX "beyond_max.dlx.obj"
#define BEYOND_ALLOCATED "beyond_allocated.dlx.obj"
#define GROW_STACK "grow_stack.dlx.obj"
#define HELLO_100 "hello_100.dlx.obj"
#define PROCESS_SPAWN "process_spawn.dlx.obj"

void main (int argc, char *argv[])
{
  int testcase_idx = 0;             // Used to store number of processes to create
  int i;                               // Loop index variable
  sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
  char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes
  char process_str[50];

  // Printf("[DBG] Start makeprocs!\n");

  if (argc != 2) {
    Printf("Usage: %s <test case index>\n", argv[0]);
    Printf("Index options: 0: Hello World\n");
    Printf("               1: Beyond max virtual address\n");
    Printf("               2: Beyond allocated pages\n");
    Printf("               3: Grow user stack to more than 1 page\n");
    Printf("               4: Create 100 processes sequentially to print Hello World\n");
    Printf("               5: Spawn 30 processes to run in parallel\n");
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  testcase_idx = dstrtol(argv[1], NULL, 10); // the "10" means base 10


  Printf("makeprocs (%d): Creating test case processes: %d\n", getpid(), testcase_idx);

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.
  if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  // Setup the command-line arguments for the new processes.  We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);

  // Create test processes
  Printf("-------------------------------------------------------------------------------------\n");
  switch (testcase_idx) {
  case 0:
    Printf("makeprocs (%d): Creating hello world process\n", getpid());
    process_create(HELLO_WORLD, s_procs_completed_str, NULL);
    break;
  case 1:
    Printf("makeprocs (%d): Creating process which will access beyond max virtual address\n", getpid());
    process_create(BEYOND_MAX, s_procs_completed_str, NULL);
    break;
  case 2:
    Printf("makeprocs (%d): Creating process which will access beyond allocated pages\n", getpid());
    process_create(BEYOND_ALLOCATED, s_procs_completed_str, NULL);
    break;
  case 3:
    Printf("makeprocs (%d): Creating process which will grow user stack to more than 1 page\n", getpid());
    process_create(GROW_STACK, s_procs_completed_str, NULL);
    break;

  default:
    Printf("[ERROR] Test index is out-of-boundary\n", argv[0]);
    Printf("Index options: 0: Hello World\n");
    Printf("               1: Beyond max virtual address\n");
    Printf("               2: Beyond allocated pages\n");
    Printf("               3: Grow user stack to more than 1 page\n");
    Printf("               4: Create 100 processes sequentially to print Hello World\n");
    Printf("               5: Spawn 30 processes to run in parallel\n");
    Printf("-------------------------------------------------------------------------------------\n");
    Exit();
  }

  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }
  

  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

}
