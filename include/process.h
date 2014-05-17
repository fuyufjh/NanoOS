#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "adt/list.h"

#define PCB_POOL_SIZE 2048
#define KSTACK_SIZE 4096

#define STAT_READY 0
#define STAT_SLEEPING 1
typedef unsigned proc_stat_t;

ListHead ready, block, free;

typedef struct Semaphore {
	int token;
	ListHead block;
} Sem;


typedef union PCB {
    uint32_t kstack[KSTACK_SIZE];
    struct {
        ListHead list;
        void *tf;
        /* ABOVE ITEMS SHOULD NOT BE MODIFIED!  */
        int locked;
        proc_stat_t stat;
        ListHead msg_list;
        Sem msg_sem;
        pid_t pid;
        CR3 cr3;
    };
} PCB;

extern PCB *current;
PCB* create_kthread(void *fun, ...);

void sleep(void);
void wakeup(PCB *p);
void sleep_sem(Sem*);

PCB pcb_pool[PCB_POOL_SIZE];
uint32_t num_of_proc;

void lock();
void unlock();
void P(Sem* s);
void V(Sem* s);
void create_sem(Sem* sem, int value);

unsigned lock_flag;

typedef struct Message {
	pid_t src, dest;
	union {
		int type;
		int ret;
	};
	union {
		int i[5];
		struct {
			pid_t req_pid;
			int dev_id;
			void *buf;
			off_t offset;
			size_t len;
		};
	};
	ListHead list;
} Msg;

/* Message */
#define ANY -1
#define MSG_HARD_INTR -2
void send(pid_t dest, Msg *m);
void receive(pid_t src, Msg *m);

/* TTY */
void copy_from_kernel(PCB* pcb, void* dest, void* src, int len);
void copy_to_kernel(PCB* pcb, void* dest, void* src, int len);
void strcpy_to_kernel(PCB* pcb, char* dest, char* src);
void strcpy_from_kernel(PCB* pcb, char* dest, char* src);

inline PCB* fetch_pcb(pid_t pid);
void add_irq_handle(int irq, void (*func)(void) );

#endif
