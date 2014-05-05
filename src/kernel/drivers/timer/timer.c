#include "kernel.h"
#include "x86/x86.h"
#include "hal.h"
#include "time.h"
#include "string.h"

#define PORT_TIME 0x40
#define PORT_RTC  0x70
#define FREQ_8253 1193182

static long jiffy = 0;
static Time rt;

static void update_jiffy(void);
static void init_i8253(void);
static void init_rt(void);
static void timer_driver_thread(void);

void init_timer(void) {
	init_i8253();
	init_rt();
	add_irq_handle(0, update_jiffy);
	PCB *p = create_kthread(timer_driver_thread);
	TIMER = p->pid;
	wakeup(p);
	hal_register("timer", TIMER, 0);
    init_rt();
}


static uint32_t timer_active=0;
struct timer_struct
{
    unsigned long expire;
    pid_t pid;
} timer[32];

static inline void set_timer(int sec, pid_t pid)
{
    int test_active=0x1,usable_timer=0 , i;
    for (i=0;i<32;i++)
    {
        if (!(test_active & timer_active)) break;
        usable_timer++;
        test_active += test_active;
        if (test_active == 0) test_active=1;
    }
    assert(i<32); // If (i >= 32) Error: too many timers

    timer[usable_timer].expire = sec * HZ + jiffy;
    timer[usable_timer].pid = pid;
    timer_active |= test_active;
}

static inline void run_timers(void){
    struct timer_struct *tp = timer;
    uint32_t mask;

    for (mask=1; mask ;tp++,mask += mask)
    {
        if (mask > timer_active) break;
        if (!(mask & timer_active)) continue;
        if (tp->expire > jiffy) continue;
        timer_active &= ~mask;
        Msg m;
        m.type = TIME_UP;
        m.src = TIMER;
        send(tp->pid, &m);
    }
}

static void
timer_driver_thread(void) {
	static Msg m;

	while (true) {
		receive(ANY, &m);

		switch (m.type) {
        case NEW_TIMER:
            set_timer(m.i[0] , m.src);
            break;

        case UPDATE_TIMER:
            run_timers();
            break;

		default:
            assert(0);

		}
	}
}

long
get_jiffy() {
	return jiffy;
}

static int
md(int year, int month) {
	bool leap = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
	static int tab[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	return tab[month] + (leap && month == 2);
}

static void
update_jiffy(void) {
	jiffy ++;
	if (jiffy % HZ == 0) {
		rt.second ++;
		if (rt.second >= 60) { rt.second = 0; rt.minute ++; }
		if (rt.minute >= 60) { rt.minute = 0; rt.hour ++; }
		if (rt.hour >= 24)   { rt.hour = 0;   rt.day ++;}
		if (rt.day >= md(rt.year, rt.month)) { rt.day = 1; rt.month ++; }
		if (rt.month >= 13)  { rt.month = 1;  rt.year ++; }
	}

    Msg m;
    m.type = UPDATE_TIMER;
    send(TIMER, &m);
}

static void
init_i8253(void) {
	int count = FREQ_8253 / HZ;
	assert(count < 65536);
	out_byte(PORT_TIME + 3, 0x34);
	out_byte(PORT_TIME, count & 0xff);
	out_byte(PORT_TIME, count >> 8);
}

static inline uint8_t get_time_part(uint8_t part)
{
    out_byte(0x70, part);
    uint8_t bcd = in_byte(0x71);
    return (bcd>>4)*10 + (bcd & 0xf);
}

static void
init_rt(void) {
	memset(&rt, 0, sizeof(Time));
	/* Optional: Insert code here to initialize current time correctly */
    rt.year = get_time_part(TIME_YEAR);
    rt.month = get_time_part(TIME_MONTH);
    rt.day = get_time_part(TIME_DAY);
    rt.hour = get_time_part(TIME_DAY);
    rt.minute = get_time_part(TIME_MINUTE);
    rt.second = get_time_part(TIME_SECOND);
}

void
get_time(Time *tm) {
	memcpy(tm, &rt, sizeof(Time));
}
