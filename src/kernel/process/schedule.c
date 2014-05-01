#include "kernel.h"
#include "adt/list.h"

PCB idle, *current = &idle;

void
schedule(void) {
	/* implement process/thread schedule here */
    if (list_empty(&ready))
        current = &idle;
    else if (current == &idle)
        current=(PCB*)(ready.next);
    else if (current->ts.stat == STAT_SLEEPING)
        current=(PCB*)(ready.next);
    else
    {
        current = (PCB*)(current->ts.list.next);
        if ((ListHead*)current == &ready)
            current = (PCB*)(current->ts.list.next);
    }
    //printk("SCHDULE: PROC %d\n",current->ts.pid);
}
