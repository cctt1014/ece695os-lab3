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
// return size of a single L2 page table
int MemoryGetSizeofL2PageTable();

void MemoryIncreaseRefCounter(int);

// Find and return the first available L2 page table base address
uint32 MemroyAllocL2PageTable(void);

// Maps input virtual page index to a physical page 
// with control bits set accrodingly
uint32 MemorySetupPte (uint32 page);

// Allocate ppage and set status bits for pte
uint32 MemorySetPte(uint32 ppagenum);

// Find and return the first available ppage frame number
uint32 MemoryAllocPage(void);

// Free up L2 page table based on its base address
void MemoryFreeL2Table(uint32 addr);

// Free up physical pages based on ppage number
void MemoryFreePage(uint32 page);

// ROP access violation handler
int MemoryROPAccessHandler(PCB * pcb);

#endif	// _memory_h_
