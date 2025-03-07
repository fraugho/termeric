#ifndef SPRITE_HPP
#define SPRITE_HPP

#include "screen.hpp"
#include "input.hpp"

void sprite_render(Buffer* buf) {
    //screen.load_file_clean("sprite.spt", buf);
    //screen.load_file_print("sprite.spt", buf);
    screen.load_file_print("circle.spt", buf, {1, 1});
    screen.load_file_clean("circle.spt", buf, {1, 1});
}
/*
 * */

#endif // !SPRITE_HPP
