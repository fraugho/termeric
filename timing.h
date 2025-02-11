#ifndef TIMING_H
#define TIMING_H
#include <time.h>
#include <stdint.h>

/* Timing utilities */
static inline int64_t get_ms() {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC_RAW, &spec);
    return (int64_t)spec.tv_sec * 1000LL + (int64_t)spec.tv_nsec / 1000000LL;
}

static inline int64_t get_us() {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC_RAW, &spec);
    // Convert seconds to microseconds first, then add nanoseconds converted to microseconds
    return (int64_t)spec.tv_sec * 1000000LL + (int64_t)spec.tv_nsec / 1000LL;
}

// Helper function to compute time difference safely
static inline int64_t time_diff_us(int64_t start, int64_t end) {
    return end - start;
}

static inline int64_t get_ns() {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC_RAW, &spec);
    return (int64_t)spec.tv_sec * 1000000000LL + (int64_t)spec.tv_nsec;
}

static inline int64_t time_diff_ns(int64_t start, int64_t end) {
    return end - start;
}

// Utility functions for conversions
static inline double ns_to_ms(int64_t ns) {
    return ns / 1000000.0;
}

static inline double ns_to_us(int64_t ns) {
    return ns / 1000.0;
}

static inline double ns_to_fps(int64_t ns) {
    return 1000000000.0 / ns;
}

// Timer structure for reusable interval checking
typedef struct {
    int64_t last_time;
    int64_t interval;
} Timer;

static inline void reset_timer(Timer* timer, int64_t current_time) {
    timer->last_time = current_time;
}

static inline bool is_time_elapsed(const Timer* timer, int64_t current_time) {
    return (current_time - timer->last_time) >= timer->interval;
}
#endif
