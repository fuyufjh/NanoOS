#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "adt/list.h"

#define PCB_POOL_SIZE 2048
#define KSTACK_SIZE 4096

#define STAT_READY 0
#define STAT_SLEEPING 1
typedef unsigned proc_stat_t;

ListHead ready, block, free;

struct task_struct {
    ListHead list;
    void *tf;
    int locked;
    proc_stat_t stat;
};

typedef union PCB {
    uint32_t kstack[KSTACK_SIZE];
    struct task_struct ts;
} PCB;

extern PCB *current;

void sleep(void);
void wakeup(PCB *p);
void sleep_sem(ListHead* block);

PCB pcb_pool[PCB_POOL_SIZE];
uint32_t num_of_proc;

typedef struct Semaphore {
	int token;
	ListHead block;
} Sem;

void lock();
void unlock();
void P(Sem* s);
void V(Sem* s);
void create_sem(Sem* sem, int value);

unsigned lock_flag;

#endif
