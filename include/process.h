#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "adt/list.h"

#define PCB_POOL_SIZE 2048
#define KSTACK_SIZE 4096

#define STAT_WAITING 0
#define STAT_WORKING 1
#define STAT_SLEEPING 2
typedef unsigned proc_stat_t;

ListHead ready, block, free;

struct task_struct {
    ListHead list;
    void *tf;
    //proc_stat_t stat;
};

typedef union PCB {
    uint32_t kstack[KSTACK_SIZE];
    struct task_struct ts;
} PCB;

extern PCB *current;

void sleep(void);
void wakeup(PCB *p);

PCB pcb_pool[PCB_POOL_SIZE];
uint32_t num_of_proc;

#endif
