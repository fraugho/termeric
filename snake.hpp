#ifndef SNAKE_HPP
#define SNAKE_HPP

#include <time.h>
#include "screen.hpp"
#include "engine.hpp"
#include "vec.hpp"

const uint8_t INIT_SIZE = 5;

class Snake{
    public:
        Vec<int16_t> x;
        Vec<int16_t> y;
        int8_t vx;
        int8_t vy;
        uint8_t size;
        int16_t apple_x;
        int16_t apple_y;
        uint8_t y_move = 0;

        void init(){
            size = INIT_SIZE;

            vx = 1;
            vy = 0;

            for(int i = INIT_SIZE; i > 0; --i){
                x.append(i);
            }

            for(int i = 0; i < INIT_SIZE; ++i){
                y.append(50);
            }

            apple_x = 50;
            apple_y = 50;
        }

        void move(int key) {
            switch (key) {
                //up
                case 'w':
                    vx = 0;
                    vy = -1;
                    y_move = 1;
                    break;
                    //left
                case 'a':
                    vx = -1;
                    vy = 0;
                    y_move = 0;
                    break;
                    //down
                case 's':
                    vx = 0;
                    vy = 1;
                    y_move = 1;
                    break;
                    //right
                case 'd':
                    vx = 1;
                    vy = 0;
                    y_move = 0;
                    break;
                default:
                    break;
            }
        }

        void move_head(){
            x[0] += vx;
            y[0] += vy;

            // Wrap around the screen
            if (x[0] < 0){
                x[0] = screen.width;
            }
            if (x[0] > screen.width){
                x[0] = 0;
            }
            if (y[0] < 0){
                y[0] = screen.height;
            }
            if (y[0] > screen.height){
                y[0] = 0;
            }
        }

        void apple_logic(){
            if (apple_x == x[0] &&
                    apple_y == y[0]){

                x.append(0);
                y.append(0);
                ++size;

                //changes apple position
                apple_x = rand() % screen.width;
                apple_y = rand() % screen.height;
            }
        }

        void collision_logic(){
            for(int i = 1; i < size; ++i){
                if (x[0] == x[i] && y[0] == y[i]){
                    engine_close();
                }
            }
        }

        void move() {
            collision_logic();
            apple_logic();

            // Store old head position
            int16_t prev_x = x[0];
            int16_t prev_y = y[0];

            // Move head first
            move_head();

            // Move rest of body - each segment takes previous segment's position
            for(int i = 1; i < size; ++i) {
                int16_t temp = x[i];
                x[i] = prev_x;
                prev_x = temp;

                temp = y[i];
                y[i] = prev_y;
                prev_y = temp;
            }
        }

        // Draw snake
        void render(Buffer* buf) {
            for(int i = 0; i < size; ++i) {
                screen.draw_pixel(buf, (Point){x[i], y[i]}, '#');
            }
        }

        void clean(Buffer* buf) {
            for(int i = 0; i < size; ++i) {
                screen.draw_pixel(buf, (Point){x[i], y[i]}, ' ');
            }
        }

        void apple_render(Buffer* buf) {
            int center_y = screen.height / 2;
            screen.draw_pixel(buf, (Point){apple_x, apple_y}, '*');
        }
    private:
};



#endif
