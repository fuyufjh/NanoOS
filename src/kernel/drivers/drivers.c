#include "hal.h"

void init_hal();
void init_timer();
void init_tty();
void init_ide();
void init_mem();
void init_kmem();
void init_null();
void init_zero();
void init_random();
void init_ramdisk();
void init_fm();

void init_driver() {
	init_hal();
	init_timer();
	init_tty();
	init_ide();
    init_mem();
    init_kmem();
    init_null();
    init_zero();
    init_random();
    init_ramdisk();
    init_fm();

	hal_list();
}
