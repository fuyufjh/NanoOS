#include "hal.h"

void init_hal();
void init_timer();
void init_tty();
void init_ide();
void init_ram();
void init_kmem();
void init_null();
void init_zero();

void init_driver() {
	init_hal();
	init_timer();
	init_tty();
	init_ide();
    init_ram();
    init_kmem();
    init_null();
    init_zero();

	hal_list();
}
