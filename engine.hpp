#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <pthread.h>
#include <time.h>
#include <thread>
//mine
#include "timing.hpp"
#include "screen.hpp"
#include "init.hpp"
#include "input.hpp"
#include "int.hpp"
#include "config.hpp"

int times = 0;
i64 total = 0;
i64 min_time = INT64_MAX;
i64 max_time = 0;

int io_index = 0;
int render_index = 0;

bool RUNNING = true;

int remaining_threads = 0;

void render();

i64 RENDER_INTERVAL = 100000;

static inline void engine_close();
void* thread_render(void* args) {
    while (RUNNING) {
#if FRAME_CAPPING_ENABLED
        static Timer render_timer = {0, RENDER_INTERVAL};
        i64 current_time = get_us();
        if (is_time_elapsed(&render_timer, current_time) && screen.frames[render_index].state == RENDER) {
        reset_timer(&render_timer, current_time);
#else
        if (screen.frames[render_index].state == RENDER) {
#endif
            render();
            screen.frames[render_index].state = IO;
            render_index = (render_index + 1) % num_frames;
            if (get_key() == CTRL_KEY('q')) {
                engine_close();
            }
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

void engine_run(){
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

    for(auto thread : threads){
        pthread_join(thread, NULL);
    }

    screen.free();
    print_perf();
}

void thread_engine_run(){
    screen.init();

    const int BASIC_TASK_TC = 2;

    std::thread threads[BASIC_TASK_TC];

    //std::thread thread = std::thread(thread_render);
    //threads[0] = std::thread(thread_render);
    //threads[1] = std::thread(thread_write);

    long cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
    remaining_threads += cpu_num - 2;

    while (RUNNING) {
        last_key = editor_read_key();
    }

    for(auto& thread : threads){
        thread.join();
    }

    screen.free();
    print_perf();
}

static inline void engine_close(){
    RUNNING = false;
}
#endif
