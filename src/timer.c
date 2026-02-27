#include "timer.h"

#include <stdio.h>
#include <time.h>

typedef struct {
  int tic;
  int sec;
  int min;
  int hour;
} TimeComponents;

static struct timespec start_ts;
static struct timespec prev_ts;
static int timer_active = 0;

static double get_elapsed_seconds(struct timespec start, struct timespec end) {
  return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

static void format_time(double total_seconds, TimeComponents* tc) {
  long total_cents = (long)(total_seconds * 100);
  tc->hour = total_cents / 360000L;
  total_cents %= 360000L;
  tc->min = total_cents / 6000;
  total_cents %= 6000;
  tc->sec = total_cents / 100;
  tc->tic = total_cents % 100;
}

void timer_start(void) {
  clock_gettime(CLOCK_MONOTONIC, &start_ts);
  prev_ts = start_ts;
  timer_active = 1;
}

void print_time(const char* message) {
  struct timespec current_ts;
  clock_gettime(CLOCK_MONOTONIC, &current_ts);

  if (!timer_active) {
    timer_start();
    return;
  }

  TimeComponents total_tc, stage_tc;
  format_time(get_elapsed_seconds(start_ts, current_ts), &total_tc);
  format_time(get_elapsed_seconds(prev_ts, current_ts), &stage_tc);

  printf("Time: total=%2.2d:%2.2d:%2.2d.%2.2d, %s=%2.2d:%2.2d:%2.2d.%2.2d\n", total_tc.hour,
         total_tc.min, total_tc.sec, total_tc.tic, message, stage_tc.hour, stage_tc.min,
         stage_tc.sec, stage_tc.tic);

  prev_ts = current_ts;
}

/* Maintain legacy API for compatibility */
void print_full_time(const char* message) { print_time(message); }
void TimerStart(void) { timer_start(); }
long TimerGet(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (long)(ts.tv_sec * 100 + ts.tv_nsec / 10000000);
}

long PrintTime(const char* message) {
  struct timespec current_ts;
  clock_gettime(CLOCK_MONOTONIC, &current_ts);
  double stage_sec = get_elapsed_seconds(prev_ts, current_ts);
  print_time(message);
  return (long)(stage_sec * 100);
}

void sprint_time(char* buffer) {
  struct timespec current_ts;
  clock_gettime(CLOCK_MONOTONIC, &current_ts);
  TimeComponents tc;
  format_time(get_elapsed_seconds(start_ts, current_ts), &tc);
  sprintf(buffer, "%2.2d:%2.2d:%2.2d.%2.2d", tc.hour, tc.min, tc.sec, tc.tic);
}

long int PrintTimeT(const char* message, long int* pTotalTime) {
  struct timespec current_ts;
  clock_gettime(CLOCK_MONOTONIC, &current_ts);
  double total_sec = get_elapsed_seconds(start_ts, current_ts);
  double stage_sec = get_elapsed_seconds(prev_ts, current_ts);
  print_time(message);
  *pTotalTime = (long)(total_sec * 100);
  return (long)(stage_sec * 100);
}
