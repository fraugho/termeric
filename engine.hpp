#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <pthread.h>
#include <time.h>
//mine
#include "timing.h"
#include "screen.hpp"
#include "init.hpp"
#include "input.hpp"

int times = 0;
int64_t total = 0;
int64_t min_time = INT64_MAX;
int64_t max_time = 0;

int io_index = 0;
int render_index = 0;

bool RUNNING = true;

int remaining_threads = 0;

void render();

void* thread_render(void* args){
    while(RUNNING){
        if (screen.frames[render_index].state == RENDER) {
            render();
            screen.frames[render_index].state = IO;
            render_index = (render_index + 1) % num_frames;
        }
    }
    return NULL;
}

/* Writing */
void* thread_write(void* args) {
    long start = get_ns();
    while(RUNNING){
        if (screen.frames[io_index].state == IO){
            if (screen.frames[io_index].used != 0){
                screen.write_to_terminal(&screen.frames[io_index]);
            }

            screen.frames[io_index].used = 0;
            screen.frames[io_index].state = RENDER;
            io_index = (io_index + 1) % num_frames;

            // Calculate and display timing
            int64_t current = get_ns();
            int64_t elapsed_ns = time_diff_ns(start, current);

            if (elapsed_ns > 0) {
                total += elapsed_ns;
                min_time = (elapsed_ns < min_time) ? elapsed_ns : min_time;
                max_time = (elapsed_ns > max_time) ? elapsed_ns : max_time;
                ++times;
            }

            start = get_ns();
        }
    }
    return NULL;
}

void print_perf(){
    printf("\n\r\x1b[K");
    printf("Performance Stats:\n\x1b[K\r");
    if (times > 0) {
        double avg_time = total / (double)times;
        //avg_time = (elasped - main_start) / (double)times;
        printf("Average frame time: %.2f ns (%.2f FPS)\n\x1b[K\r",
                avg_time, ns_to_fps(avg_time));
        printf("Best frame time: %ld ns (%.2f FPS)\n\x1b[K\r", 
                min_time, ns_to_fps(min_time));
        printf("Worst frame time: %ld ns (%.2f FPS)\n\x1b[K\r", 
                max_time, ns_to_fps(max_time));
    }
}

void engine_init(){
    screen.init();

    const int BASIC_TASK_TC = 2;

    pthread_t threads[BASIC_TASK_TC];

    pthread_create(&threads[0], NULL, thread_render, NULL);
    pthread_create(&threads[1], NULL, thread_write, NULL);

    long cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
    remaining_threads += cpu_num - 2;

    while (RUNNING) {
        last_key = editor_read_key();
    }


    for(int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }

    screen.free();
    print_perf();
}

static inline void engine_close(){
    RUNNING = false;
}
#endif
