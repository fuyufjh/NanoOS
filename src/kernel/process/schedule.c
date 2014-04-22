#include "kernel.h"
#include "adt/list.h"

PCB idle, *current = &idle;

void
schedule(void) {
	/* implement process/thread schedule here */
    if (list_empty(&ready)) current = &idle;
    else if (current == &idle) current=(PCB*)(ready.next);
    else
    {
        if (((struct task_struct*)current)->stat == STAT_SLEEPING)
            current=(PCB*)(ready.next);
        else
        {
            current = (PCB*)((ListHead*)current)->next;
            if ((ListHead*)current == &ready) current = (PCB*)((ListHead*)current)->next;
        }
        //if (current < pcb_pool || current > &pcb_pool[PCB_POOL_SIZE-1])
        //    current=(PCB*)(ready.next);
    }

    //printk("SCHDULE: PROC %d\n",current-&pcb_pool[0]);
}
