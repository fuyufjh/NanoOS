#ifndef __PROCESS_H__
#define __PROCESS_H__

#define PCB_POOL_SIZE 2048
#define KSTACK_SIZE 4096
typedef struct PCB {
	void *tf;
    uint32_t kstack[KSTACK_SIZE];
} PCB;

extern PCB *current;

PCB pcb_pool[PCB_POOL_SIZE];
uint32_t num_of_proc;

#endif
