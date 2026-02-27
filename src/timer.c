/* Time and Timing Functions */
#include <stdio.h>

#include "timer.h"
/* To measure time on different platforms, several timing functions are provided.
   The specific function used depends on the available libraries.
   If MEASURE_FULL_TIME is defined, wall-clock time is also measured. */
#define MEASURE_FULL_TIME

/* Select which function to use for CPU time measurement. */
#define USE_getrusage
//#define USE_times
//#define USE_clock

/* Select which function to use for wall-clock time measurement. */
#define USE_gettimeofday

#ifdef USE_getrusage
#include <sys/time.h>
#include <sys/resource.h>

#ifdef __hpux
#include <sys/syscall.h>
#define getrusage(a,b) syscall(SYS_getrusage,a,b)
#endif

/* Returns CPU time in hundredths of a second.
   This version measures user time (system time is not included). */
static long int get_time (void)
{
  struct rusage buf;

  getrusage (RUSAGE_SELF, &buf);
  return   buf.ru_utime.tv_sec * 100            // Full seconds converted to hundredths
         + buf.ru_utime.tv_usec / 10000;        // Microseconds converted to hundredths
}
#endif /* USE_getrusage */

#ifdef USE_times
#include <time.h>
#include <sys/times.h>

/* Returns CPU time in hundredths of a second.
   This version measures user time (system time is not included). */
static long int get_time (void)
{
  struct tms buf;

  times (&buf);

  return buf.tms_utime / (CLK_TCK / 100);       // CPU ticks converted to hundredths
}
#endif /* USE_times */

#ifdef USE_clock
#include <time.h>

/* Returns CPU time in hundredths of a second.
   This version measures user time + system time. */
static long int get_time (void)
{
  long int t;

  t = (long int) clock ();

  return t / (CLOCKS_PER_SEC / 100);            // clock ticks converted to hundredths
}
#endif /* USE_clock */

#ifdef USE_gettimeofday
#include <sys/time.h>

/* Returns wall-clock time in hundredths of a second. */
static long int get_full_time (void)
{
  struct timeval buf;

  gettimeofday (&buf, 0);
  return   buf.tv_sec * 100
	 + buf.tv_usec / 10000;
}
#endif /* USE_gettimeofday */

typedef struct _timer_
{
  int  tic;
  int  sec;
  int  min;
  int  hour;
} TIMER;

/* Converts time in hundredths of a second to HH:MM:SS.CC format. */
static void ConvertTime (long clocks, TIMER *t)
{
  t->hour = clocks/360000L;
  clocks %= 360000L;
  t->min  = clocks/6000;
  clocks %= 6000;
  t->sec  = clocks/100;
  t->tic  = clocks%100;
}

static int TimerStarted;        // 1 if timer is active
static long int StartTime;      // Start CPU time
static long int PrevTime;       // CPU time at previous call to get_time

#ifdef MEASURE_FULL_TIME
static long int StartFullTime;	// Start wall-clock time
static long int PrevFullTime;	// Wall-clock time at previous call to get_time
#endif /* MEASURE_FULL_TIME */

/* Starts the timer. */
void timer_start (void)
{
  TimerStarted = 1;
  StartTime = PrevTime = get_time ();
#ifdef MEASURE_FULL_TIME
  StartFullTime = PrevFullTime = get_full_time ();
#endif /* MEASURE_FULL_TIME */
}

/* Prints CPU time elapsed since start and since previous call. */
void print_time (const char *message)
{
  long t;
  TIMER summ, stage;

  t = get_time ();

  if (TimerStarted)
    {
      ConvertTime (t - StartTime, &summ);
      ConvertTime (t - PrevTime, &stage);
      printf("Time: total=%2.2d:%2.2d:%2.2d.%2.2d, %s=%2.2d:%2.2d:%2.2d.%2.2d\n",
              summ.hour, summ.min, summ.sec, summ.tic, message,
              stage.hour, stage.min, stage.sec, stage.tic);
      PrevTime = t;
    }
  else
    {
      TimerStarted = 1;
      StartTime = PrevTime = t;
    }
}

/* Prints both CPU and wall-clock time elapsed. */
void print_full_time (const char *message)
{
  long t;
  TIMER summ, stage;

#ifdef MEASURE_FULL_TIME
  TIMER summ_full, stage_full;
  long t_full = get_full_time ();
#endif /* MEASURE_FULL_TIME */

  t = get_time ();

  if (TimerStarted)
    {
      ConvertTime (t - StartTime, &summ);
      ConvertTime (t - PrevTime, &stage);
#ifdef MEASURE_FULL_TIME
      ConvertTime (t_full - StartFullTime, &summ_full);
      ConvertTime (t_full - PrevFullTime, &stage_full);
#endif /* MEASURE_FULL_TIME */
#ifdef MEASURE_FULL_TIME
      printf("Time: total=%2.2d:%2.2d:%2.2d.%2.2d (%2.2d:%2.2d:%2.2d.%2.2d), %s=%2.2d:%2.2d:%2.2d.%2.2d (%2.2d:%2.2d:%2.2d.%2.2d)\n",
	      summ.hour, summ.min, summ.sec, summ.tic,
	      summ_full.hour, summ_full.min, summ_full.sec, summ_full.tic,
	      message,
	      stage.hour, stage.min, stage.sec, stage.tic,
	      stage_full.hour, stage_full.min, stage_full.sec, stage_full.tic);
      PrevFullTime = t_full;
#else
      printf("Time: total=%2.2d:%2.2d:%2.2d.%2.2d, %s=%2.2d:%2.2d:%2.2d.%2.2d\n",
	      summ.hour, summ.min, summ.sec, summ.tic, message,
	      stage.hour, stage.min, stage.sec, stage.tic);
#endif /* MEASURE_FULL_TIME */
      PrevTime = t;
    }
  else
    {
      TimerStarted = 1;
      StartTime = PrevTime = t;
#ifdef MEASURE_FULL_TIME
      StartFullTime = PrevFullTime = t_full;
#endif /* MEASURE_FULL_TIME */
    }
}

/* Starts the timer. */
void TimerStart (void)
{
  timer_start ();
}

/* Prints CPU time and returns time elapsed since previous call. */
long PrintTime (const char *message)
{
  long t;
  TIMER summ, stage;
  long res;

  t = get_time ();

  if (TimerStarted)
    {
      ConvertTime (t - StartTime, &summ);
      ConvertTime (res = t - PrevTime, &stage);
      printf("Time: total=%2.2d:%2.2d:%2.2d.%2.2d, %s=%2.2d:%2.2d:%2.2d.%2.2d\n",
              summ.hour, summ.min, summ.sec, summ.tic, message,
              stage.hour, stage.min, stage.sec, stage.tic);
      PrevTime = t;
    }
  else
    {
      TimerStarted = 1;
      StartTime = PrevTime = t;
      res = 0;
    }
  return res;
}

/* Formats current total elapsed time into buffer. */
void sprint_time (char *buffer)
{
  long t;
  TIMER summ;

  t = get_time ();

  if (TimerStarted)
    {
      ConvertTime (t - StartTime, &summ);
      sprintf (buffer, "%2.2d:%2.2d:%2.2d.%2.2d",
               summ.hour, summ.min, summ.sec, summ.tic);
    }
  else
    {
      TimerStarted = 1;
      StartTime = PrevTime = t;
    }
}

/* Prints CPU time and returns total elapsed time. */
long int PrintTimeT (const char *message, long int * pTotalTime)
{
  long t;
  TIMER summ, stage;
  long res;

  t = get_time ();

  if (TimerStarted)
    {
      ConvertTime (t - StartTime, &summ);
      ConvertTime (res = t - PrevTime, &stage);
      printf("Time: total=%2.2d:%2.2d:%2.2d.%2.2d, %s=%2.2d:%2.2d:%2.2d.%2.2d\n",
	      summ.hour, summ.min, summ.sec, summ.tic, message,
	      stage.hour, stage.min, stage.sec, stage.tic);
      PrevTime = t;
      *pTotalTime = t - StartTime;
    }
  else
    {
      TimerStarted = 1;
      StartTime = PrevTime = t;
      res = 0;
    }
  return res;
}

/* Returns CPU time since start. */
long TimerGet (void)
{
  return get_time ();
}
