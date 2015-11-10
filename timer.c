// AANGEPAST VOOR MAC

#include <time.h>
#include "timer.h"

#include <sys/time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

void current_utc_time(struct timespec *ts) {

#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  ts->tv_sec = mts.tv_sec;
  ts->tv_nsec = mts.tv_nsec;
#else
  clock_gettime(CLOCK_REALTIME, ts);
#endif

}

struct timespec start_time;

/*
 * Starts the timing. Get a result by calling timer_end afterwards.
 */
void timer_start(void)
{	
	current_utc_time(&start_time);
}

/*
 * Returns the elapsed time since calling timer_start in seconds.
 * Results when calling this without calling timer_end are undefined.
 */
double timer_end(void)
{
    struct timespec end_time;
    current_utc_time(&end_time);

    return difftime(end_time.tv_sec, start_time.tv_sec) +
        (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.;
}


