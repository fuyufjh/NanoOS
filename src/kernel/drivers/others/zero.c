#include "kernel.h"
#include "hal.h"

pid_t DEV_ZERO;

void memset(void *dest, uint8_t data, size_t size);
static void
zero_driver_thread(void) {
    static Msg m;

    while (true) {
        receive(ANY, &m);

        if (m.type == DEV_READ) {
            memset(m.buf, 0, m.len);
            m.ret = m.len;
            m.dest = m.src;
            m.src = DEV_ZERO;
            send(m.dest, &m);
        } else if (m.type == DEV_WRITE) {
            m.ret = 0;
            m.dest = m.src;
            m.src = DEV_ZERO;
            send(m.dest, &m);
        } else assert(0);
    }
}

void
init_zero(void) {
    PCB* p=create_kthread(zero_driver_thread);
    DEV_ZERO = p->pid;
    hal_register("zero", DEV_ZERO, 0);
    wakeup(p);
}
