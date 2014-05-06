#include "kernel.h"
#include "hal.h"
#include "time.h"

pid_t RANDOM;

static uint8_t next;
Time rt;

static inline uint8_t
rand() {
    next = next * 1103515245 + 12345;
    return next;
}

static inline void
srand() {
    get_time(&rt);
    next = rt.hour*3600 + rt.minute*60 + rt.second;
    rand();
}

static void
random_driver_thread(void) {
    static Msg m;

    while (true) {
        receive(ANY, &m);

        if (m.type == DEV_READ) {
            int i;
            for (i=0;i<m.len;i++)
                *((uint8_t*)m.buf + i)=rand();
            m.ret = m.len;
            m.dest = m.src;
            m.src = RANDOM;
            send(m.dest, &m);
        } else if (m.type == DEV_WRITE) {
            m.ret = 0;
            m.dest = m.src;
            m.src = RANDOM;
            send(m.dest, &m);
        } else assert(0);
    }
}

void
init_random(void) {
    PCB* p=create_kthread(random_driver_thread);
    RANDOM = p->pid;
    hal_register("random", RANDOM, 0);
    srand();
    wakeup(p);
}
