#include "kernel.h"
#include <stdarg.h>
#include "adt/list.h"

PCB pcb_pool[PCB_POOL_SIZE] align_to_page;

#define MAX_NUM_OF_ARGUMENTS 16

PCB*
create_kthread(void *fun, ...) {
    cli();

    assert(free.next != &free);
    PCB* selected_free = (PCB*)free.next;
    list_del((ListHead*)selected_free);

    uint32_t* frame = (void*)(selected_free) + KSTACK_SIZE;

    void** p = &fun + MAX_NUM_OF_ARGUMENTS;
    int i;
    for (i=0;i<MAX_NUM_OF_ARGUMENTS;i++)
        *(frame--) = (uint32_t)*(p--);

    frame[-1]=0x202; //eflags
    asm("mov %%cs, %0":"=a"(frame[-2])); //cs
    frame[-3]=(uint32_t)fun; //eip
    //frame[-4]=0; //error_code
    //frame[-5]=0; //irq
    asm("mov %%ds, %0":"=a"(frame[-6])); //ds
    asm("mov %%es, %0":"=a"(frame[-7])); //es
    asm("mov %%fs, %0":"=a"(frame[-8])); //fs
    asm("mov %%gs, %0":"=a"(frame[-9])); //gs
    //frame[-10]=0; //eax
    //frame[-11]=0; //ecx
    //frame[-12]=0; //edx
    //frame[-13]=0; //ebx
    //frame[-14]=0; //esp
    //frame[-15]=0; //ebp
    //frame[-16]=0; //esi
    //frame[-17]=0; //edi
    ((struct task_struct *)selected_free)->tf = &frame[-17];
    list_add_before(&block, (ListHead*)selected_free);
    ((struct task_struct*)selected_free)->locked=0;
    ((struct task_struct*)selected_free)->stat = STAT_SLEEPING;
    sti();

    return selected_free;
}

void A();
void B();
void C();
void D();
PCB* PCB_of_thread_A;
PCB* PCB_of_thread_B;
PCB* PCB_of_thread_C;
PCB* PCB_of_thread_D;
void test_setup(void);

void
init_proc() {
    list_init(&ready);
    list_init(&block);
    list_init(&free);

    int i;
    for (i=0;i<PCB_POOL_SIZE;i++)
        list_add_before(&free, &(pcb_pool[i].ts.list));

    // test
    //PCB_of_thread_A=create_kthread(A);
    //PCB_of_thread_B=create_kthread(B);
    //PCB_of_thread_C=create_kthread(C);
    //PCB_of_thread_D=create_kthread(D);
    //wakeup(PCB_of_thread_A);

}

void A () {
    int x = 0;
    while(1) {
        if(x % 100000 == 0) {
            printk("a");
            wakeup(PCB_of_thread_B);
            sleep();
        }
        x ++;
    }
}
void B () {
    int x = 0;
    while(1) {
        if(x % 100000 == 0) {
            printk("b");
            wakeup(PCB_of_thread_C);
            sleep();
        }
        x ++;
    }
}
void C () {
    int x = 0;
    while(1) {
        if(x % 100000 == 0) {
            printk("c");
            wakeup(PCB_of_thread_D);
            sleep();
        }
        x ++;
    }
}
void D () {
    int x = 0;
    while(1) {
        if(x % 100000 == 0) {
            printk("d");
            wakeup(PCB_of_thread_A);
            sleep();
        }
        x ++;
    }
}
extern int lock_count;
void sleep(void)
{
    //cli();
    //((struct task_struct*)current)->locked=lock_count;
    lock();
    list_del((ListHead*)current);
    list_add_before(&block,(ListHead*)current);
    ((struct task_struct*)current)->stat = STAT_SLEEPING;
    unlock();
    asm volatile("int $0x80");
}
void sleep_sem(ListHead* block)
{
    //cli();
    //((struct task_struct*)current)->locked=lock_count;
    lock();
    list_del((ListHead*)current);
    list_add_before(block,(ListHead*)current);
    ((struct task_struct*)current)->stat = STAT_SLEEPING;
    unlock();
    asm volatile("int $0x80");
}
void wakeup(PCB *p)
{
    assert(((struct task_struct*)p)->stat == STAT_SLEEPING);
    //cli();
    lock();
    list_del((ListHead*)p);
    list_add_after(&ready,(ListHead*)p);
    ((struct task_struct *)p)->stat = STAT_READY;
    unlock();
    //sti();
}


/* TEST CODE *//* TEST CODE */

#define NBUF 5
#define NR_PROD 3
#define NR_CONS 4
int buf[NBUF], f = 0, r = 0, g = 1;
int last = 0;
Sem empty, full, mutex;

void
test_producer(void) {
	while (1) {
		P(&empty);
		P(&mutex);
		if(g % 10000 == 0) {
			printk(".");	// tell us threads are really working
		}
		buf[f ++] = g ++;
		f %= NBUF;
		V(&mutex);
		V(&full);
	}
}

void
test_consumer(void) {
	int get;
	while (1) {
		P(&full);
		P(&mutex);
		get = buf[r ++];
		assert(last == get - 1);	// the products should be strictly increasing
		last = get;
		r %= NBUF;
		V(&mutex);
		V(&empty);
	}
}

void
test_setup(void) {
	create_sem(&full, 0);
	create_sem(&empty, NBUF);
	create_sem(&mutex, 1);
	int i;
	for(i = 0; i < NR_PROD; i ++) {
		wakeup(create_kthread(test_producer));
	}
	for(i = 0; i < NR_CONS; i ++) {
		wakeup(create_kthread(test_consumer));
	}
}
