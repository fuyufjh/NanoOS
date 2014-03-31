#include "kernel.h"

PCB idle, *current = &idle;

void
schedule(void) {
	/* implement process/thread schedule here */
    if (current == &idle) current=&pcb_pool[0];
    else
    {
        current++;
        while (current->stat == STAT_SLEEPING)
        {
            current++;
            if (current == &pcb_pool[num_of_proc]) current=&pcb_pool[0];
        }
        if (current == &pcb_pool[num_of_proc]) current=&pcb_pool[0];
    }
}
