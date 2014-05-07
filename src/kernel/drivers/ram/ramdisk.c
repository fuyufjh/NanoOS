#include "kernel.h"
#include "hal.h"

void memcpy(void *, const void *, size_t);

#define NR_MAX_FILE 8
#define NR_FILE_SIZE (128 * 1024)

static uint8_t file[NR_MAX_FILE][NR_FILE_SIZE] = {
	//{0x12, 0x34, 0x56, 0x78},
    {"This place is prepared for the first created file.\n"},
	{"-- I use flename to locate files.\n"},
    {"Hello, world!\n"},
	//{0x7f, 0x45, 0x4c, 0x46}
};
static uint8_t *disk = (void*)file;

pid_t RAMDISK;

static void
ram_driver_thread(void) {
    static Msg m;

    while (true) {
        receive(ANY, &m);

        if (m.src == MSG_HARD_INTR) {
            panic("RAMDISK unexpected hard interrupt");
        } else if (m.type == DEV_READ) {
            memcpy(m.buf, (void*)(disk + m.offset), m.len);
            m.ret = m.len;
            m.dest = m.src;
            m.src = RAMDISK;
            send(m.dest, &m);
        } else if (m.type == DEV_WRITE) {
            memcpy((void*)(disk + m.offset), m.buf, m.len);
            m.ret = m.len;
            m.dest = m.src;
            m.src = RAMDISK;
            send(m.dest, &m);

        } else assert(0);
    }
}

void
init_ramdisk(void) {
    PCB* p=create_kthread(ram_driver_thread);
    RAMDISK = p->pid;
    hal_register("ram", RAMDISK, 0);
    wakeup(p);
}
