#include "kernel.h"
#include "hal.h"

pid_t DEV_NULL;

static void
null_driver_thread(void) {
    static Msg m;

    while (true) {
        receive(ANY, &m);

        if (m.type == DEV_READ) {
            m.ret = 0;
            m.dest = m.src;
            m.src = DEV_NULL;
            send(m.dest, &m);
        } else if (m.type == DEV_WRITE) {
            m.ret = 0;
            m.dest = m.src;
            m.src = DEV_NULL;
            send(m.dest, &m);
        } else assert(0);
    }
}

void
init_null(void) {
    PCB* p=create_kthread(null_driver_thread);
    DEV_NULL = p->pid;
    hal_register("null", DEV_NULL, 0);
    wakeup(p);
}
