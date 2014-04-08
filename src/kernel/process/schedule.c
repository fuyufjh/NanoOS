#include "kernel.h"
#include "adt/list.h"

PCB idle, *current = &idle;

void
schedule(void) {
	/* implement process/thread schedule here */
    cli();
    if (list_empty(&ready))
    {
        current = &idle;
        return;
    }
    if (current == &idle) current=(PCB*)(ready.next);
    else current = (PCB*)(((ListHead*)current)->next);
    //if ((ListHead*)current == &ready) current=(PCB*)(ready.next);
    if (current < pcb_pool || current > &pcb_pool[PCB_POOL_SIZE-1])
        current=(PCB*)(ready.next);
    //printk("SCHDULE: PROC %d, s=%d\n",current-&pcb_pool[0],((struct task_struct*)current)->stat);
    sti();
}
