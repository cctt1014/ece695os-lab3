//
//	memory.c
//
//	Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

#include "ostraps.h"
#include "dlxos.h"
#include "../include/os/process.h"
#include "../include/os/memory.h"
#include "queue.h"

// num_pages = size_of_memory / size_of_one_page
static uint32 freemap[MEM_FREEMAP_LENGTH/*size*/];
static int ref_counter[MEM_NUM_PHYS_PAGE];
static uint32 pagestart;
static int nfreepages;
static int freemapmax;

// Static area for 256 L2 pages
static uint32 l2_page_table_list[256][MEM_L2TABLE_SIZE];
static int l2_table_assigned[256];

//----------------------------------------------------------------------
//
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//
//----------------------------------------------------------------------
static int negativeone = 0xFFFFFFFF;
static inline uint32 invert (uint32 n) {
  return (n ^ negativeone);
}

//----------------------------------------------------------------------
//
//	MemoryGetSize
//
//	Return the total size of memory in the simulator.  This is
//	available by reading a special location.
//
//----------------------------------------------------------------------
int MemoryGetSize() {
  return (*((int *)DLX_MEMSIZE_ADDRESS));
}

int MemoryGetSizeofL2PageTable() {
  return sizeof(l2_page_table_list[0]);
}

void MemoryIncreaseRefCounter(int ppagenum) {
  ref_counter[ppagenum]++;
}


//----------------------------------------------------------------------
//
//	MemoryInitModule
//
//	Initialize the memory module of the operating system.
//      Basically just need to setup the freemap for pages, and mark
//      the ones in use by the operating system as "VALID", and mark
//      all the rest as not in use.
//
//----------------------------------------------------------------------
void MemoryModuleInit() {
  int i, j;
  int curr_addr;

  for (i = 0; i < MEM_FREEMAP_LENGTH; i++) {
    freemap[i] = 0;
    for (j = 0; j < 32; j++) {
      curr_addr = (i << 17) + (j << 12);
      if (curr_addr <= lastosaddress) {
        freemap[i] = freemap[i] | (1 << j);
      } else {
        // printf("[DBG] End of freemap init. i=%d, j=%d, lastosaddress=%x\n", i, j, lastosaddress);
        return;
      }
    }
  }

  for (i = 0; i < MEM_NUM_PHYS_PAGE; i++) {
    ref_counter[i] = 0;
  }

  for (i = 0; i < 256; i++) {
    l2_table_assigned[i] = 0;
    for (j = 0; j < MEM_L2TABLE_SIZE; j++) {
      l2_page_table_list[i][j] = 0;
    }
  }
}


//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//	Translate a user address (in the process referenced by pcb)
//	into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr) {
  uint32 l1_vpage_num, l2_vpage_num, offset, ppage_num;
  uint32 *l2_table_base_ptr;

  l1_vpage_num = addr >> MEM_L1FIELD_FIRST_BITNUM;
  l2_table_base_ptr = (uint32*) (pcb->pagetable[l1_vpage_num]);
  l2_vpage_num = (addr >> MEM_L2FIELD_FIRST_BITNUM) % MEM_L2TABLE_SIZE;
  ppage_num =  *(l2_table_base_ptr + l2_vpage_num) / MEM_PAGESIZE;
  offset    = addr & (uint32)MEM_PAGEOFFSET_MASK;

  return (ppage_num*MEM_PAGESIZE + offset);
}


//----------------------------------------------------------------------
//
//	MemoryMoveBetweenSpaces
//
//	Copy data between user and system spaces.  This is done page by
//	page by:
//	* Translating the user address into system space.
//	* Copying all of the data in that page
//	* Repeating until all of the data is copied.
//	A positive direction means the copy goes from system to user
//	space; negative direction means the copy goes from user to system
//	space.
//
//	This routine returns the number of bytes copied.  Note that this
//	may be less than the number requested if there were unmapped pages
//	in the user range.  If this happens, the copy stops at the
//	first unmapped address.
//
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir) {
  unsigned char *curUser;         // Holds current physical address representing user-space virtual address
  int		bytesCopied = 0;  // Running counter
  int		bytesToCopy;      // Used to compute number of bytes left in page to be copied

  while (n > 0) {
    // Translate current user page to system address.  If this fails, return
    // the number of bytes copied so far.
    curUser = (unsigned char *)MemoryTranslateUserToSystem (pcb, (uint32)user);

    // If we could not translate address, exit now
    if (curUser == (unsigned char *)0) break;

    // Calculate the number of bytes to copy this time.  If we have more bytes
    // to copy than there are left in the current page, we'll have to just copy to the
    // end of the page and then go through the loop again with the next page.
    // In other words, "bytesToCopy" is the minimum of the bytes left on this page 
    // and the total number of bytes left to copy ("n").

    // First, compute number of bytes left in this page.  This is just
    // the total size of a page minus the current offset part of the physical
    // address.  MEM_PAGESIZE should be the size (in bytes) of 1 page of memory.
    // MEM_ADDRESS_OFFSET_MASK should be the bit mask required to get just the
    // "offset" portion of an address.
    bytesToCopy = MEM_PAGESIZE - ((uint32)curUser & MEM_ADDRESS_OFFSET_MASK);
    
    // Now find minimum of bytes in this page vs. total bytes left to copy
    if (bytesToCopy > n) {
      bytesToCopy = n;
    }

    // Perform the copy.
    if (dir >= 0) {
      bcopy (system, curUser, bytesToCopy);
    } else {
      bcopy (curUser, system, bytesToCopy);
    }

    // Keep track of bytes copied and adjust addresses appropriately.
    n -= bytesToCopy;           // Total number of bytes left to copy
    bytesCopied += bytesToCopy; // Total number of bytes copied thus far
    system += bytesToCopy;      // Current address in system space to copy next bytes from/into
    user += bytesToCopy;        // Current virtual address in user space to copy next bytes from/into
  }
  return (bytesCopied);
}

//----------------------------------------------------------------------
//
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//
//----------------------------------------------------------------------
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, from, to, n, 1));
}

int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, to, from, n, -1));
}

//---------------------------------------------------------------------
// MemoryPageFaultHandler is called in traps.c whenever a page fault 
// (better known as a "seg fault" occurs.  If the address that was
// being accessed is on the stack, we need to allocate a new page 
// for the stack.  If it is not on the stack, then this is a legitimate
// seg fault and we should kill the process.  Returns MEM_SUCCESS
// on success, and kills the current process on failure.  Note that
// fault_address is the beginning of the page of the virtual address that 
// caused the page fault, i.e. it is the vaddr with the offset zero-ed
// out.
//
// Note: The existing code is incomplete and only for reference. 
// Feel free to edit.
//---------------------------------------------------------------------
int MemoryPageFaultHandler(PCB *pcb) {
  uint32 vpagenum, ppagenum, stackpagenum;
  uint32 fault_address;
  uint32 *l2_table_base_ptr;

  dbprintf('m', "Starting page fault handler\n");

  // stack pointer will be adjusted first when malloc new memory space
  // -8 is applied due to previous frame pointer and the return address 
  // (4 bytes each) which are copied to the stack immediately after
  // entering the function but before modifying the stack pointer
  stackpagenum = (pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]-8) >> MEM_L2FIELD_FIRST_BITNUM;

  fault_address = pcb->currentSavedFrame[PROCESS_STACK_FAULT];
  vpagenum = fault_address >> MEM_L2FIELD_FIRST_BITNUM;

  // segfault if the faulting address is not part of the stack
  if (vpagenum < stackpagenum) {
    dbprintf('m', "fault_address = %x\nsp = %x\n", fault_address, pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]);
    printf("[FATAL] (%d): segmentation fault at page address %x\n", findpid(pcb), fault_address);
    ProcessKill();
    return MEM_FAIL;
  }

  ppagenum = MemoryAllocPage();
  l2_table_base_ptr = (uint32*) pcb->pagetable[vpagenum/MEM_L2TABLE_SIZE];
  *(l2_table_base_ptr + (vpagenum%MEM_L2TABLE_SIZE)) = MemorySetPte(ppagenum);
  dbprintf('m', "Returning from page fault handler. vpage=%d is mapped to ppage=%d\n", vpagenum, ppagenum);
  return MEM_SUCCESS;
}

int MemoryROPAccessHandler(PCB * pcb) {
  uint32 vpagenum, ppagenum;
  uint32 l1_vpagenum, l2_vpagenum;
  uint32 *l2_table_base_ptr;
  uint32 *old_ppage_base_ptr;
  uint32 *new_ppage_base_ptr;

  vpagenum = pcb->currentSavedFrame[PROCESS_STACK_FAULT] >> MEM_L2FIELD_FIRST_BITNUM;
  dbprintf('m', "MemoryROPAccessHandler Start!\n");

  l1_vpagenum = vpagenum / MEM_L2TABLE_SIZE;
  l2_vpagenum = vpagenum % MEM_L2TABLE_SIZE;

  l2_table_base_ptr = (uint32 *) pcb->pagetable[l1_vpagenum];
  ppagenum = *(l2_table_base_ptr + l2_vpagenum) / MEM_PAGESIZE;
  // save the old ppage base address
  old_ppage_base_ptr = (uint32 *) (ppagenum * MEM_PAGESIZE);

  if (ref_counter[ppagenum] > 1) { // at least 2 processes using the ppage
    *(l2_table_base_ptr + l2_vpagenum) = MemorySetPte(MemoryAllocPage());
    new_ppage_base_ptr = (uint32 *)(*(l2_table_base_ptr + l2_vpagenum) & MEM_PTE_MASK);

    // copy the shared page to the new location
    bcopy(old_ppage_base_ptr, new_ppage_base_ptr, MEM_PAGESIZE);

    ref_counter[ppagenum]--;
  } else if (ref_counter[ppagenum] == 1) { // exact 1 process using the ppage
    *(l2_table_base_ptr + l2_vpagenum) &= ~(MEM_PTE_READONLY);
  } else {
    printf("[ERROR %d] ref_counter error detected in MemoryROPAccessHandler\n", findpid(pcb));
    printf("[ERROR %d] ref_counter[ppagenum] = %d\n", findpid(pcb), ref_counter[ppagenum]);
    return;
  }

  dbprintf('m', "MemoryROPAccessHandler Done!\n");

}
//---------------------------------------------------------------------
// You may need to implement the following functions and access them from process.c
// Feel free to edit/remove them
//---------------------------------------------------------------------

// Find and return the first available L2 page table base address
uint32 MemroyAllocL2PageTable(void) {
  int i;

  for (i = 0; i < 256; i++) {
    if (l2_table_assigned[i] == 0) {
      l2_table_assigned[i] = 1;
      return (uint32) &l2_page_table_list[i][0];
    }
  }

  printf("[ERROR %d] No more l2 page tables available for use.\n", GetCurrentPid());
  ProcessKill(currentPCB);
  return -1;

}


// Find and return the first available ppage frame number
uint32 MemoryAllocPage(void) {
  int i, j;

  for (i = 0; i < MEM_FREEMAP_LENGTH; i++) {
    // printf("[DBG] freemap[%d]=%x\n",i, freemap[i]);
    for (j = 0; j < 32; j++) {
      if ((freemap[i] & (uint32)(1 << j)) == 0) {
        freemap[i] |= (1 << j);
        // printf("[DBG] Page allocation return ppage num = %d with i=%d, j=%d", (i << 5) + (j), i,j);
        ref_counter[i*32+j]++;
        return (i << 5) + (j);
      }
    }
  }

  printf("[ERROR %d] No more free physical page in free map.\n", GetCurrentPid());
  ProcessKill(currentPCB);
  return -1;
}


// Maps input virtual page index to an initial physical page 
// with control bits masked accrodingly
uint32 MemorySetupPte (uint32 page) {
  return 0 & MEM_PTE_MASK;
}

// Allocate ppage and set status bits for L2 pte
uint32 MemorySetPte(uint32 ppagenum) {
  return (ppagenum * MEM_PAGESIZE) | MEM_PTE_VALID;
}

// Free up L2 page table based on its base address
void MemoryFreeL2Table(uint32 addr) {
  uint32 overall_base;
  uint32 *table_ptr;
  int idx;
  int i;

  overall_base = (uint32) &l2_page_table_list[0][0];
  idx = (addr - overall_base) / sizeof(l2_page_table_list[0]);

  for (i = 0; i < MEM_L2TABLE_SIZE; i++) {
    if ((l2_page_table_list[idx][i] & (uint32)(MEM_PTE_VALID)) != 0 ) {
      MemoryFreePage((l2_page_table_list[idx][i] & MEM_PTE_MASK)/MEM_PAGESIZE);
      l2_page_table_list[idx][i] = 0;
    }
  }

  l2_table_assigned[idx] = 0;
}

// Free up physical pages based on ppage number
void MemoryFreePage(uint32 page) {
  int i, j;

  ref_counter[page]--;

  if (ref_counter[page] > 0) {
    return;
  } else if (ref_counter[page] == 0) {
    i = page >> 5;
    j = page % 32;

    freemap[i] &= ~(1 << j);
  } else {
    printf("[ERROR %d] ref_counter error detected in MemoryFreePage\n", findpid(currentPCB));
    printf("[ERROR %d] ref_counter[ppagenum] = %d\n", findpid(currentPCB), ref_counter[page]);
    return;
  }

  
}

