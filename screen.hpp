#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <fstream>

#include "init.hpp"
#include "buffer.h"
#include "int.hpp"

#define RENDER 1 << 1
#define IO 1 << 2

typedef struct Vec2{
    i16 x;
    i16 y;
} Vec2;

class Screen{
    public:
        Buffer* frames;
        u16 width, height;

        void draw_blank(){
            char* buf = (char*)malloc(width * height);
            memset(buf, ' ', width * height);
            write(STDOUT_FILENO, buf, width * height);
        }

        void init() {
            enable_raw_mode();
            if (get_window_size(&height, &width) == -1){
                die("get_window_size");
            }
            height -= debug;

            draw_blank();

            // Hide cursor
            write(STDOUT_FILENO, "\x1b[?25l", 6);

            // Initialize both buffers
            frames = (Buffer*)malloc(sizeof(Buffer) * num_frames);

            for (int i = 0; i < num_frames; i++){
                frames[i].c = (char*)malloc(BUF_SIZE);
                frames[i].len = BUF_SIZE;
                frames[i].used = 0;
                frames[i].state = RENDER;
            }
        }

        void clean_square(Buffer* buf, Vec2 TopLeft, Vec2 BottomRight, char c) {
            size_t total_len = 0;  // This will store the total length of the output string
            static const char* format = "\x1b[%hd;%hdH%c";  // Escape sequence format

            // Calculate how much space we will need in the buffer
            for (u16 y = TopLeft.y; y <= BottomRight.y; ++y) {
                for (u16 x = TopLeft.x; x <= BottomRight.x; ++x) {
                    total_len += snprintf(NULL, 0, format, y, x, c);  // Add the length for each formatted string
                }
            }

            // Make sure the buffer has enough space for the total string
            if (total_len > buf->len - buf->used) {
                return;
            }

            char* cursor = &buf->c[buf->used];

            for (u16 y = TopLeft.y; y <= BottomRight.y; ++y) {
                for (u16 x = TopLeft.x; x <= BottomRight.x; ++x) {
                    cursor += snprintf(cursor, buf->len - (cursor - buf->c), format, y, x, c);
                }
            }

            // Update the buffer used size
            buf->used += total_len;
        }

        static inline void write_to_terminal(Buffer* buf) {
            write(STDOUT_FILENO, buf->c, buf->used);
        }

        static inline void draw_pixel(Buffer* buf, Vec2 point, char c){
            u16 len = snprintf(&buf->c[buf->used], buf->len - buf-> used, "\x1b[%hd;%hdH%c", point.y, point.x, c); buf->used += len;
        }

        void load_file_print(std::string file_name, Buffer* buf, Vec2 pos){
            std::ifstream file(file_name);
            std::string text;
            //draw_pixel(buf, {0, 0}, '#');
            //draw_pixel(buf, {0,  1}, '#');
            i16 x = pos.x;
            while(std::getline(file, text)){
                u8 len = text.length();
                for(int i = 0; i < len; ++i, ++pos.x){
                    draw_pixel(buf, pos, text[i]);
                }
                ++pos.y;
                pos.x = x;
            }
        }

        void load_file_clean(std::string file_name, Buffer* buf, Vec2 pos){
            std::ifstream file(file_name);
            std::string text;
            i16 x = pos.x;
            while(std::getline(file, text)){
                u8 len = text.length();
                for(int i = 0; i < len; ++i, ++pos.x){
                    draw_pixel(buf, pos, ' ');
                }
                ++pos.y;
                pos.x = x;
            }
        }

        void draw_line(){
        }

        void free(){
            disable_raw_mode();
            for(int i = 0; i < num_frames; ++i){
                buf_free(&frames[i]);
            }
        }
    private:
};

/* Global state */
Screen screen;

#endif
