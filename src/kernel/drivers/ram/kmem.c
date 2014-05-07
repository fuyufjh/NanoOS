#include "kernel.h"
#include "hal.h"

void memcpy(void *, const void *, size_t);

pid_t DEV_KMEM;

static void
kmem_driver_thread(void) {
    static Msg m;

    while (true) {
        receive(ANY, &m);

        if (m.src == MSG_HARD_INTR) {
            panic("DEV_KMEM unexpected hard interrupt");
        } else if (m.type == DEV_READ) {
            memcpy(m.buf, (void*)m.offset + KOFFSET, m.len);
            m.ret = m.len;
            m.dest = m.src;
            m.src = DEV_KMEM;
            send(m.dest, &m);
        } else if (m.type == DEV_WRITE) {
            memcpy((void*)m.offset + KOFFSET, m.buf, m.len);
            m.ret = m.len;
            m.dest = m.src;
            m.src = DEV_KMEM;
            send(m.dest, &m);

        } else assert(0);
    }
}

void
init_kmem(void) {
    PCB* p=create_kthread(kmem_driver_thread);
    DEV_KMEM = p->pid;
    hal_register("kmem", DEV_KMEM, 0);
    wakeup(p);
}
