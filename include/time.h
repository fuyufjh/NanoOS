#ifndef __TIME_H__
#define __TIME_H__

#define HZ        100

#define TIME_SECOND  0x0
#define TIME_MINUTE  0x2
#define TIME_HOUR    0x4
#define TIME_DAY     0x7
#define TIME_MONTH   0x8
#define TIME_YEAR    0x9

#define NEW_TIMER 101
#define TIME_UP 102
#define UPDATE_TIMER 103

pid_t TIMER;

typedef struct Time {
	int year, month, day;
	int hour, minute, second;
} Time;
inline long get_jiffy();

void get_time(Time *tm);

#endif
