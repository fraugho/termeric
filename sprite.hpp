#ifndef SPRITE_HPP
#define SPRITE_HPP

#include "screen.hpp"
#include "input.hpp"
#include "timing.hpp"

#define RENDER_INTERVAL 100000

void sprite_render(Buffer* buf) {
    static i16 x = 1;
    static Timer render_timer = {0, RENDER_INTERVAL};

    i64 current_time = get_us();
    static i8 stage = 0;
    static bool back = true;
    // Render
    if (is_time_elapsed(&render_timer, current_time)) {
        switch(stage % 3){
            case 0:
                screen.load_file_clean("sprites/p.spt", buf, {i16(x - 1), 1});
                screen.load_file_print("sprites/p.spt", buf, {x, 1});
                back = true;
                stage = 1;
                break; 
            case 1:
                screen.load_file_clean("sprites/pac.spt", buf, {i16(x - 1), 1});
                screen.load_file_print("sprites/pac.spt", buf, {x, 1});
                stage = back ? 2 : 0;
                break;
            case 2:
                screen.load_file_clean("sprites/pc.spt", buf, {i16(x - 1), 1});
                screen.load_file_print("sprites/pc.spt", buf, {x, 1});
                stage = 1;
                back = false;
                break;
        }
        x %= 50;
        ++x;
        reset_timer(&render_timer, current_time);
    }
}
/*
 * */

#endif // !SPRITE_HPP
