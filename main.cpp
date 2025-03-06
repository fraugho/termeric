/* Terminator - A simple terminal graphics system */
/* Standard includes */
//mine
#include "timing.hpp"
#include "snake.hpp"
#include "engine.hpp"
#include "input.hpp"

//global snake
Snake snake;

// Constants for intervals
#define X_RENDER_INTERVAL 50000
#define Y_RENDER_INTERVAL 100000
#define MOVE_INTERVAL 10000

void render() {
    static Timer render_timer = {0, X_RENDER_INTERVAL};
    static Timer move_timer = {0, MOVE_INTERVAL};

    static i64 last_key_check = 0;
    i64 current_time = get_us();

    // Adjust render interval dynamically because of terminal cell width to height ratio
    render_timer.interval = snake.y_move ? Y_RENDER_INTERVAL : X_RENDER_INTERVAL;

    // Update logic
    if (is_time_elapsed(&move_timer, current_time)) {
        snake.move(last_key);  // Move snake based on input
        reset_timer(&move_timer, current_time);
    }

    // Render
    if (is_time_elapsed(&render_timer, current_time)) {
        snake.clean(&screen.frames[render_index]);
        snake.move();
        snake.render(&screen.frames[render_index]);
        snake.apple_render(&screen.frames[render_index]);
        reset_timer(&render_timer, current_time);
    }

    // Handle user input
    if (get_key() == CTRL_KEY('q')) {
        engine_close();
    }
}

/* Main entry point */
int main() {
    snake.init();
    engine_run();
}
