#ifndef SNAKE_HPP
#define SNAKE_HPP

#include <time.h>
#include "screen.hpp"
#include "engine.hpp"
#include "vec.h"

const uint8_t INIT_SIZE = 5;

class Snake{
    public:
        Vec* x;
        Vec* y;
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

            x = vec_create(INIT_SIZE, sizeof(int16_t));
            for(int i = INIT_SIZE; i > 0; --i){
                int16_t num = i;
                vec_append(x, &num);
            }

            y = vec_create(INIT_SIZE, sizeof(int16_t));
            for(int i = 0; i < INIT_SIZE; ++i){
                int16_t num = 50;
                vec_append(y, &num);
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

            ((int16_t*)x->data)[0] += vx;
            ((int16_t*)y->data)[0] += vy;

            // Wrap around the screen
            if (((int16_t*)x->data)[0] < 0){
                ((int16_t*)x->data)[0] = screen.width;
            }
            if (((int16_t*)x->data)[0] > screen.width){
                ((int16_t*)x->data)[0] = 0;
            }
            if (((int16_t*)y->data)[0] < 0){
                ((int16_t*)y->data)[0] = screen.height;
            }
            if (((int16_t*)y->data)[0] > screen.height){
                ((int16_t*)y->data)[0] = 0;
            }
        }

        void apple_logic(){
            if (apple_x == ((int16_t*)x->data)[0] &&
                    apple_y == ((int16_t*)y->data)[0]){

                static int16_t num = 0;
                //adds body cell
                vec_append(x, &num);
                vec_append(y, &num);
                ++size;

                //changes apple position
                apple_x = rand() % screen.width;
                apple_y = rand() % screen.height;
            }
        }

        void collision_logic(){
            for(int i = 1; i < size; ++i){
                if (((int16_t*)x->data)[0] == ((int16_t*)x->data)[i] &&
                        ((int16_t*)y->data)[0] == ((int16_t*)y->data)[i]){
                    engine_close();
                }
            }
        }

        void move() {
            collision_logic();
            apple_logic();

            // Store old head position
            int16_t prev_x = ((int16_t*)x->data)[0];
            int16_t prev_y = ((int16_t*)y->data)[0];

            // Move head first
            move_head();

            // Move rest of body - each segment takes previous segment's position
            for(int i = 1; i < size; ++i) {
                int16_t temp = ((int16_t*)x->data)[i];
                ((int16_t*)x->data)[i] = prev_x;
                prev_x = temp;

                temp = ((int16_t*)y->data)[i];
                ((int16_t*)y->data)[i] = prev_y;
                prev_y = temp;
            }
        }

        // Draw snake
        void render(Buffer* buf) {
            for(int i = 0; i < size; ++i) {
                screen.draw_pixel(buf, (Point){((int16_t*)x->data)[i], ((int16_t*)y->data)[i]}, '#');
            }
        }

        void clean(Buffer* buf) {
            for(int i = 0; i < size; ++i) {
                screen.draw_pixel(buf, (Point){((int16_t*)x->data)[i], ((int16_t*)y->data)[i]}, ' ');
            }
        }

        void apple_render(Buffer* buf) {
            int center_y = screen.height / 2;
            screen.draw_pixel(buf, (Point){apple_x, apple_y}, '*');
        }

        void free(){
            free_vec(x);
            free_vec(y);
        }
    private:
};



#endif
