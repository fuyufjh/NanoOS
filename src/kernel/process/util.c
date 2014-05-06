#include "kernel.h"
#include <stdarg.h>
#include "adt/list.h"

PCB pcb_pool[PCB_POOL_SIZE] align_to_page;

#define MAX_NUM_OF_ARGUMENTS 16

PCB*
create_kthread(void *fun, ...) {
    int intr_flag = read_eflags() & 0x200;
    cli();

    assert(free.next != &free);
    PCB* selected_free = list_entry(free.next, PCB, list);
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
    selected_free->tf = &frame[-17];

    list_add_before(&block, &selected_free->list);
    selected_free->locked=0;
    selected_free->stat = STAT_SLEEPING;
    list_init(&selected_free->msg_list);
    create_sem(&selected_free->msg_sem,0);
    selected_free->pid = selected_free - pcb_pool;

    if (intr_flag) sti();

    return selected_free;
}

void test_setup(void);
pid_t pidA;
pid_t pidB;
pid_t pidC;
pid_t pidD;
pid_t pidE;

void
init_proc() {
    list_init(&ready);
    list_init(&block);
    list_init(&free);

    int i;
    for (i=1;i<PCB_POOL_SIZE;i++) // do not use pcb_pool[0]
        list_add_before(&free, &(pcb_pool[i].list));

}

void A () {
	Msg m1, m2;
	m1.src = current->pid;
	int x = 0;
	while(1) {
		if(x % 10000000 == 0) {
			printk("a");
			send(pidE, &m1);
			receive(pidE, &m2);
		}
		x ++;
	}
}
void B () {
	Msg m1, m2;
	m1.src = current->pid;
	int x = 0;
	receive(pidE, &m2);
	while(1) {
		if(x % 10000000 == 0) {
			printk("b");
			send(pidE, &m1);
			receive(pidE, &m2);
		}
		x ++;
	}
}
void C () {
	Msg m1, m2;
	m1.src = current->pid;
	int x = 0;
	receive(pidE, &m2);
	while(1) {
		if(x % 10000000 == 0) {
			printk("c");
			send(pidE, &m1);
			receive(pidE, &m2);
		}
		x ++;
	}
}
void D () {
	Msg m1, m2;
	m1.src = current->pid;
	receive(pidE, &m2);
	int x = 0;
	while(1) {
		if(x % 10000000 == 0) {
			printk("d");
			send(pidE, &m1);
			receive(pidE, &m2);
		}
		x ++;
	}
}

void E () {
	Msg m1, m2;
	m2.src = current->pid;
	char c;
	while(1) {
		receive(ANY, &m1);
		if(m1.src == pidA) {c = '|'; m2.dest = pidB; }
		else if(m1.src == pidB) {c = '/'; m2.dest = pidC;}
		else if(m1.src == pidC) {c = '-'; m2.dest = pidD;}
		else if(m1.src == pidD) {c = '\\';m2.dest = pidA;}
		else assert(0);

		printk("\033[s\033[1000;1000H%c\033[u", c);
		send(m2.dest, &m2);
	}

}

extern int lock_count;
void sleep(void)
{
    //((struct task_struct*)current)->locked=lock_count;
    lock();
    list_del(&current->list);
    list_add_before(&block,&current->list);
    current->stat = STAT_SLEEPING;
    unlock();
    asm volatile("int $0x80");
}
void sleep_sem(Sem* s)
{
    //((struct task_struct*)current)->locked=lock_count;
    lock();
    list_del(&current->list);
    list_add_before(&s->block,&current->list);
    current->stat = STAT_SLEEPING;
    unlock();
    asm volatile("int $0x80");
}

void wakeup(PCB *p)
{
    /* Some mystrious bug happened sometimes. but WHY????
     * I use this following line to prevent it....*/
    if (p < &pcb_pool[0] || p > &pcb_pool[PCB_POOL_SIZE]) return;
    assert(p->stat == STAT_SLEEPING);
    lock();
    list_del(&p->list);
    list_add_before(&ready,&p->list);
    p->stat = STAT_READY;
    unlock();
}

inline PCB* fetch_pcb(pid_t pid)
{
    return &pcb_pool[pid];
}

/* TEST CODE */

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
