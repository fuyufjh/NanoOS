#include "kernel.h"
#include <stdarg.h>

PCB pcb_pool[PCB_POOL_SIZE];
uint32_t num_of_proc = 0;

#define MAX_NUM_OF_ARGUMENTS 16

PCB*
create_kthread(void *fun, ...) {
    uint32_t* frame = (void*)(pcb_pool[num_of_proc].kstack) + KSTACK_SIZE;

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
    pcb_pool[num_of_proc].tf = &frame[-17];

    return &pcb_pool[num_of_proc++];
}

void A();
void B();
void C();
void D();
PCB* PCB_of_thread_A;
PCB* PCB_of_thread_B;
PCB* PCB_of_thread_C;
PCB* PCB_of_thread_D;

void print_ch (int);

void
init_proc() {
    //create_kthread(A);
    //create_kthread(B);
    /*int i;
    for(i = 0; i < 7; i ++) {
        create_kthread(print_ch, 'a' + i);
    }*/
    cli();
    PCB_of_thread_A=create_kthread(A);
    PCB_of_thread_B=create_kthread(B);
    PCB_of_thread_C=create_kthread(C);
    PCB_of_thread_D=create_kthread(D);
    sti();
}
/*
void A () {
    int x = 0;
    while(1) {
        if(x % 100000 == 0) {printk("a");}
        x ++;
    }
}
void B () {
    int x = 0;
    while(1) {
        if(x % 100000 == 0) {printk("b");}
        x ++;
    }
}
*/
void print_ch (int ch) {
    int x = 0;
    while(1) {
        if(x % 100000 == 0) {printk("%c", ch);}
        x ++;
    }
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
void sleep(void)
{
    current->stat = STAT_SLEEPING;
    wait_intr();
}


void wakeup(PCB *p)
{
    p->stat = STAT_WAITING;
    //need to switch????
}

