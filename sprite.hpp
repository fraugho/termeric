#ifndef SPRITE_HPP
#define SPRITE_HPP

#include "screen.hpp"

void sprite_render(Buffer* buf) {
    static i16 x = 1;

    static bool back = true;
    // Render
    switch(x % 4){
        case 0:
            screen.load_file_clean("sprites/pac.spt", buf, {i16(x - 1), 1});
            screen.load_file_print("sprites/p.spt", buf, {x, 1});
            back = true;
            break; 
        case 1:
            screen.load_file_clean("sprites/p.spt", buf, {i16(x - 1), 1});
            screen.load_file_print("sprites/pac.spt", buf, {x, 1});
            break;
        case 2:
            screen.load_file_clean("sprites/pac.spt", buf, {i16(x - 1), 1});
            screen.load_file_print("sprites/pc.spt", buf, {x, 1});
            back = false;
            break;
        case 3:
            screen.load_file_clean("sprites/pc.spt", buf, {i16(x - 1), 1});
            screen.load_file_print("sprites/pac.spt", buf, {x, 1});
            break;
    }
    x %= 50;
    ++x;
}

#endif // !SPRITE_HPP
