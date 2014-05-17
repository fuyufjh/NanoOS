#include "kernel.h"

/* Structure of a ELF binary header */
struct ELFHeader {
	unsigned int   magic;
	unsigned char  elf[12];
	unsigned short type;
	unsigned short machine;
	unsigned int   version;
	unsigned int   entry;
	unsigned int   phoff;
	unsigned int   shoff;
	unsigned int   flags;
	unsigned short ehsize;
	unsigned short phentsize;
	unsigned short phnum;
	unsigned short shentsize;
	unsigned short shnum;
	unsigned short shstrndx;
};

/* Structure of program header inside ELF binary */
struct ProgramHeader {
	unsigned int type;
	unsigned int off;
	unsigned int vaddr;
	unsigned int paddr;
	unsigned int filesz;
	unsigned int memsz;
	unsigned int flags;
	unsigned int align;
};


#define PM_NEW_PROC 514
#define FM_NEW_FILE 1
#define FM_READ 2
#define MM_NEW_PROC 515
#define MM_NEW_PAGE 516

pid_t FM;
pid_t PM;
pid_t MM;
static uint8_t buf[512];

void create_process(int filename) {
    static Msg m;
    m.src = current->pid;
    m.type = PM_NEW_PROC;
    m.i[0] = filename;
    send(PM, &m);
    receive(PM, &m);
    printk("CREATE PROCESS DONE!\n");
}

static void do_new_proc(int filename) {
    Msg m;
    /* create file (existed) */
    m.i[0] = 1;
    m.src = PM;
    m.dest = FM;
    m.type = FM_NEW_FILE;
    send(m.dest, &m);
    receive(FM, &m);

    /* load file and read ELF */
    m.i[0] = 1;
    m.buf = buf;
    m.offset = 0;
    m.len = 512;
    m.src = PM;
    m.dest = FM;
    m.type = FM_READ;
    send(m.dest, &m);
    receive(FM, &m);

    struct ELFHeader* elf = (struct ELFHeader*)buf;
    if (elf->magic != 0x464c457fu) {
        panic("Ileague ELF file.\n");
    }

    /* create PCB */
    void* entry_point = (void*) elf->entry;
    PCB* pcb = create_kthread(entry_point);

    /* create virtual address space */
    m.src = PM;
    m.dest = MM;
    m.type = MM_NEW_PROC;
    send(m.dest, &m);
    receive(MM, &m);
    pcb->cr3.val = m.ret;
    //printk("user proc pid = %d, cr3 = %x",pcb->pid,pcb->cr3.val);

    /* Copy process to mem */
    struct ProgramHeader *ph = (struct ProgramHeader*) (((uint8_t*)elf) + elf->phoff);
    int entry;
    for (entry = 0; entry < elf->phnum; entry++) {
        m.src = pcb->pid;
        m.i[0] = ph[entry].vaddr;
        //printk("vaddr=%x\n",m.i[0]);
        m.i[1] = ph[entry].memsz;
        m.dest = MM;
        m.type = MM_NEW_PAGE;
        send(m.dest, &m);
        receive(MM, &m);
        uint8_t* pa = (uint8_t*)m.ret;
        //pa = pa_to_va(pa);
        printk("Segment %d: Physical Address: %x\n", entry , (uint32_t)pa);

        m.i[0] = 1;
        m.buf = pa;
        m.offset = ph[entry].off;
        m.len = ph[entry].filesz;
        m.src = PM;
        m.dest = FM;
        m.type = FM_READ;
        send(m.dest, &m);
        receive(FM, &m);

        uint8_t* i;
        for (i = pa + ph[entry].filesz; i < pa + ph[entry].memsz; *i ++ = 0);

    }

    wakeup(pcb);
}


void pm_thread() {
    static Msg m;
    while (true) {
        receive(ANY, &m);
        if (m.type == PM_NEW_PROC) {
            do_new_proc(m.i[0]);
            m.ret = 0;
            m.dest = m.src;
            m.src = PM;
            send(m.dest, &m);
        } else
            assert(0);
    }
}

void init_pm(void) {
    PCB* p = create_kthread(pm_thread);
    PM = p->pid;
    wakeup(p);
}
