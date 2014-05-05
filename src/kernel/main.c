#include "common.h"
#include "x86/x86.h"
#include "memory.h"
#include "process.h"

void init_page(void);
void init_serial(void);
void init_segment(void);
void init_idt(void);
void init_intr(void);
void init_proc(void);
void init_driver(void);
void welcome(void);

void os_init_cont(void);

/*void test_setup();
void A();
void B();
void C();
void D();
void E();
pid_t pidA=0;
pid_t pidB=1;
pid_t pidC=2;
pid_t pidD=3;
pid_t pidE=4;*/
void read_mbr();
void test_timer(int sec);

void
os_init(void) {
	/* Notice that when we are here, IF is always 0 (see bootloader) */

	/* We must set up kernel virtual memory first because our kernel
	   thinks it is located in 0xC0000000.
	   Before setting up correct paging, no global variable can be used. */
	init_page();

    /* Speed up the timer interrupt (for test)
    #define PORT_TIME 0x40
    #define FREQ_8253 1193182
    #define HZ        100000
    int count = FREQ_8253 / HZ;
    assert(count < 65536);
    out_byte(PORT_TIME + 3, 0x34);
    out_byte(PORT_TIME    , count % 256);
    out_byte(PORT_TIME    , count / 256);*/

	/* After paging is enabled, we can jump to the high address to keep
	 * consistent with virtual memory, although it is not necessary. */
	asm volatile (" addl %0, %%esp\n\t\
					jmp *%1": : "r"(KOFFSET), "r"(os_init_cont));

	assert(0);	// should not reach here
}

void
os_init_cont(void) {
	/* Reset the GDT. Although user processes in Nanos run in Ring 0,
	   they have their own virtual address space. Therefore, the
	   old GDT located in physical address 0x7C00 cannot be used again. */
	init_segment();

	/* Initialize the serial port. After that, you can use printk() */
	init_serial();

	/* Set up interrupt and exception handlers,
	   just as we did in the game. */
	init_idt();

	/* Initialize the intel 8259 PIC. */
	init_intr();

	/* Initialize processes. You should fill this. */
	init_proc();

	welcome();

    init_driver();

	sti();

    wakeup(create_kthread(read_mbr));

    wakeup(create_kthread(test_timer, 1));
    wakeup(create_kthread(test_timer, 2));
    wakeup(create_kthread(test_timer, 3));
    wakeup(create_kthread(test_timer, 3));

    //test_setup();

    // Test Message
    //wakeup(create_kthread(A));
    //wakeup(create_kthread(B));
    //wakeup(create_kthread(C));
    //wakeup(create_kthread(D));
    //wakeup(create_kthread(E));

	/* This context now becomes the idle process. */
	while (1) {
		wait_intr();
	}

}

void
welcome(void) {
	printk("Hello, OS World!\n");
}

/* Test IDE I/O */
#include "hal.h"
pid_t IDE;
void read_mbr()
{
    uint8_t mbr[512];
    dev_read("hda",IDE,mbr,0,512);
    printk("======= MBR Data ======\n");
    int i;
    for (i=0;i<512;i++)
        printk("%x ", mbr[i]);
    printk("\n");
    sleep();
}

/* Test Timer */
#include "time.h"
pid_t TIMER;
void test_timer(int sec)
{
    printk("Alarm was set at %d sec latter.\n",sec);
    Msg m;
    m.type = NEW_TIMER;
    m.i[0] = sec;
    send(TIMER, &m);
    receive(TIMER, &m);
    printk("Time up! (%d sec)\n", sec);
    sleep();
}
