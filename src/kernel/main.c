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

void test_setup();
void A();
void B();
void C();
void D();
void E();
pid_t pidA;
pid_t pidB;
pid_t pidC;
pid_t pidD;
pid_t pidE;
void read_mbr();
void test_timer(int sec);
void test_mem();
void test_kmem();
void test_null();
void test_zero();
void test_random();
void test_fm(int file_name, bool new_file);

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

    /* TEST CODE */
    /*wakeup(create_kthread(read_mbr));

    wakeup(create_kthread(test_timer, 1));
    wakeup(create_kthread(test_timer, 2));
    wakeup(create_kthread(test_timer, 3));
    wakeup(create_kthread(test_timer, 3));

    wakeup(create_kthread(test_ramdisk));
    wakeup(create_kthread(test_kmem));
    wakeup(create_kthread(test_null));
    wakeup(create_kthread(test_zero));
    wakeup(create_kthread(test_random));

    //test_setup();

    // Test Message
    PCB* pcbA;
    wakeup(pcbA = create_kthread(A));
    pidA = pcbA->pid;
    PCB* pcbB;
    wakeup(pcbB = create_kthread(B));
    pidB = pcbB->pid;
    PCB* pcbC;
    wakeup(pcbC = create_kthread(C));
    pidC = pcbC->pid;
    PCB* pcbD;
    wakeup(pcbD = create_kthread(D));
    pidD = pcbD->pid;
    PCB* pcbE;
    wakeup(pcbE = create_kthread(E));
    pidE = pcbE->pid;*/

    wakeup(create_kthread(test_fm, 2, true));
    wakeup(create_kthread(test_fm, 3, true));
    wakeup(create_kthread(test_fm, 4, true));
    wakeup(create_kthread(test_fm, 4, false));

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
    lock();
    for (i=0;i<512;i++)
        printk("%x ", mbr[i]);
    printk("\n");
    unlock();
    sleep();
}

/* Test Timer */
#include "time.h"
pid_t TIMER;
void test_timer(int sec)
{
    printk("Alarm was set at %d sec later.\n",sec);
    Msg m;
    m.type = NEW_TIMER;
    m.i[0] = sec;
    send(TIMER, &m);
    receive(TIMER, &m);
    printk("Time up! (%d sec)\n", sec);
    sleep();
}

/* Test Mem */
char test_data_mem[]="If you see these words, device 'mem' works well.^o^\n";
void test_mem() {
    printk("Trying to read data from mem...\n");
    static char buf[128];
    dev_read("mem", current->pid, buf, (uint32_t)test_data_mem, 127);
    printk(buf);
    sleep();
}

/* Test Kmem */
char test_data_kmem[]="If you see these words, device 'kmem' works well.^o^\n";
void test_kmem() {
    printk("Trying to read data from kmem...\n");
    static char buf[128];
    dev_read("kmem", current->pid, buf, (uint32_t)test_data_kmem - KOFFSET, 127);
    printk(buf);
    sleep();
}

/* Test NULL */
void test_null() {
    static char buf[128];
    int ret = dev_read("null", current->pid, buf, 123, 123);
    if (ret ==0) printk("If you see these words, device 'null' works well.^o^\n");
    sleep();
}

/* Test ZERO */
void memset(void *dest, uint8_t data, size_t size);
void test_zero() {
    static char buf[128];
    memset(buf, 0xff, sizeof(buf));
    dev_read("zero", current->pid, buf, 123, 120);
    if (buf[119] ==0) printk("If you see these words, device 'zero' works well.^o^\n");
    sleep();
}

/* Test RANDOM */
void test_random() {
    static char buf[4];
    memset(buf, 0xff, sizeof(buf));
    dev_read("random", current->pid, buf, 0, 4);
    printk("device 'random' gives 4 random number (range -128~127): %d %d %d %d\n",\
            buf[0], buf[1], buf[2], buf[3]);
    sleep();
}

/* Test Ramdisk */
#define FM_NEW_FILE 1
#define FM_READ 2
pid_t FM;
void test_fm(int no, bool new_file) {
    uint8_t buf[128];
    Msg m;
    printk("Trying to read from File %d...\n",no);

    if (new_file) {
        m.src = current->pid;
        m.type = FM_NEW_FILE;
        m.i[0] = no;
        send(FM, &m);
        receive(FM, &m);
    }
    m.src = current->pid;
    m.type = FM_READ;
    m.i[0] = no;
    m.buf = buf;
    m.offset = 0;
    m.len = 128;
    send(FM, &m);
    receive(FM, &m);
    printk("%sFile %d: %s",new_file?"(new) ":"" , no , buf);
    sleep();
}
