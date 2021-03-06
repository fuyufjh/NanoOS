#include "kernel.h"
#include "hal.h"

void memcpy(void *, const void *, size_t);

pid_t MEM;

static void
ram_driver_thread(void) {
    static Msg m;

    while (true) {
        receive(ANY, &m);

        if (m.src == MSG_HARD_INTR) {
            panic("MEM unexpected hard interrupt");
        } else if (m.type == DEV_READ) {
            memcpy(m.buf, (void*)m.offset, m.len);
            m.ret = m.len;
            m.dest = m.src;
            m.src = MEM;
            send(m.dest, &m);
        } else if (m.type == DEV_WRITE) {
            memcpy((void*)m.offset, m.buf, m.len);
            m.ret = m.len;
            m.dest = m.src;
            m.src = MEM;
            send(m.dest, &m);

        } else assert(0);
    }
}

void
init_mem(void) {
    PCB* p=create_kthread(ram_driver_thread);
    MEM = p->pid;
    hal_register("mem", MEM, 0);
    wakeup(p);
}
