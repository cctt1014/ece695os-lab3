#ifndef	_memory_h_
#define	_memory_h_

// Put all your #define's in memory_constants.h
#include "memory_constants.h"

extern int lastosaddress; // Defined in an assembly file

//--------------------------------------------------------
// Existing function prototypes:
//--------------------------------------------------------

int MemoryGetSize();
void MemoryModuleInit();
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr);
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir);
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryPageFaultHandler(PCB *pcb);

//---------------------------------------------------------
// Put your function prototypes here
//---------------------------------------------------------

// Maps input virtual page index to a physical page 
// with control bits set accrodingly
uint32 MemorySetupPte (uint32 page);

// Allocate ppage and set status bits for pte
uint32 MemorySetPte(uint32 ppagenum);

// Find and return the first available ppage frame number
uint32 MemoryAllocPage(void);

// Free up physical pages based on ppage number
void MemoryFreePage(uint32 page);

#endif	// _memory_h_
