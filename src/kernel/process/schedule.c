#include "kernel.h"
#include "adt/list.h"

//PCB idle, *current = &idle;
#define idle (pcb_pool[0]) // idle.pid=0
PCB *current = &idle;

void
schedule(void) {
	/* implement process/thread schedule here */
    if (list_empty(&ready))
        current = &idle;
    else if (current == &idle)
        current=(PCB*)(ready.next);
    else if (current->stat == STAT_SLEEPING)
        current=(PCB*)(ready.next);
    else
    {
        current = (PCB*)(current->list.next);
        if ((ListHead*)current == &ready)
            current = (PCB*)(current->list.next);
    }
    //printk("SCHDULE: PROC %d\n",current->pid);
}
