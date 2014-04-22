#include "kernel.h"
#include "process.h"
#include "adt/list.h"

int lock_count=0;
unsigned lock_flag=0;

void lock() {
    cli();
    lock_count++;
    //printk("%d",lock_count);
}

void unlock() {
    lock_count--;
    if (lock_flag & 0x1) return; // irq handle
    if (lock_flag & 0x2) sti(); // It is going to sleep

    if (lock_count) return;
    sti();
}

void P(Sem* s) {
    lock();
    if (s->token > 0)
    {
        s->token--;
    }
    else
    {
        //list_add_after(&(s->block), (ListHead*)current);
        lock_flag |= 0x2;
        sleep_sem(&(s->block));
        cli();
        lock_flag &= 0xfffffffd;
    }
    //printk("P%d ",s->token);
    unlock();
}

void V(Sem* s) {
    lock();
    if (!list_empty(&(s->block))) {
        wakeup((PCB*)(s->block.next));
        //list_del(s->block.next);
    } else {
        s->token++;
    }
    //printk("V%d ",s->token);
    unlock();
}

void create_sem(Sem* sem, int value)
{
    lock();
    list_init(&(sem->block));
    sem->token = value;
    unlock();
}


