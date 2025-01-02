#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>

#define true 1
#define false 0

#define CTRL_KEY(k) ((k)& 0x1f)

enum SpecialKeys{
    ESCAPE_KEY = 27,
    //Change Return key to 10 for linux
    RETURN_KEY = 13,
    BACKSPACE = 127,
    TIMEOUT_KEY = 0,
    LEFT_ARROW = 1000,
    RIGHT_ARROW,
    DOWN_ARROW,
    UP_ARROW,
    PAGE_UP,
    PAGE_DOWN,
};

struct Buffer{
    char *c;
    int len;
    int used;
};

#define BUFFER_INIT {NULL, 0}

void
buffer_append(struct Buffer *buf, char *str, int len){
    memcpy(&buf->c[buf->used], str, len);
    buf->used += len;
}

void buf_free(struct Buffer *abuf){
    free(abuf->c);
}

struct Screen{
    int width, height;
    struct termios og_termios;
    struct Buffer* frame_buffer;
};

struct Screen screen;

void
die(const char *c){
    //clears screen
    write(STDOUT_FILENO, "\x1b[2J", 4);
    //sets cursor on top left
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(c);
    exit(1);
}

void
disable_raw_mode(){
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &screen.og_termios) == -1)
        die("tcsetattr");
}

void
enable_raw_mode(){
    if (tcgetattr(STDIN_FILENO, &screen.og_termios) == -1)
        die("tcgetattr");
    atexit(disable_raw_mode);

    struct termios raw = screen.og_termios;
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_cc[VMIN] = 0;
    //sets how long it takes for read to not recive input and return 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

int
editor_read_key(){
    int nread;
    char c = 0;
    while ((nread = read(STDIN_FILENO, &c, 1)) == -1){
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    if (c == ESCAPE_KEY){
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) == -1) return ESCAPE_KEY;
        if (read(STDIN_FILENO, &seq[1], 1) == -1) return ESCAPE_KEY;

        if (seq[0] == '['){
            if (seq[1] >= 0 && seq[1] <= 9){
                if (read(STDIN_FILENO, &seq[2], 1) == -1) return ESCAPE_KEY;
                if (seq[2] == '~'){
                    switch (seq[1]) {
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                    }
                }
            }
            switch (seq[1]){
                case 'A': return UP_ARROW;
                case 'B': return DOWN_ARROW;
                case 'C': return RIGHT_ARROW;
                case 'D': return LEFT_ARROW;
            }
        }
        return ESCAPE_KEY;
    } else {
        return c;
    }
}

int
get_cursor_position(int *rows, int *cols){
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    while (i < sizeof(buf) - 1){
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

    return 0;
}

int
get_window_size(int *rows, int *cols){
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return get_cursor_position(rows, cols);
    } else{
        *cols = ws.ws_col;
        *rows= ws.ws_row;
        return 0;
    }
}

void
clear_screen(){
    // For each line we need:
    // - screen.width characters
    // - \x1b[K\r\n (5 bytes) for clear line and newline
    int line_size = screen.width + 5;
    int total_size = 3 + (line_size * screen.height);  // 3 for initial \x1b[H
    
    screen.frame_buffer = malloc(sizeof(struct Buffer));
    screen.frame_buffer->c = malloc(total_size);
    screen.frame_buffer->len = total_size;
    screen.frame_buffer->used = 0;

    // Start with cursor home
    memcpy(screen.frame_buffer->c, "\x1b[H", 3);
    screen.frame_buffer->used = 3;

    // Initialize each line with spaces and ending sequence
    char* line = malloc(line_size);
    memset(line, ' ', screen.width);
    memcpy(line + screen.width, "\x1b[K\r\n", 5);

    // Copy the formatted line screen.height times
    for (int i = 0; i < screen.height; i++) {
        memcpy(&screen.frame_buffer->c[3 + i * line_size], line, line_size);
    }

    free(line);
    screen.frame_buffer->used = total_size;
}

void screen_init() {
    if (get_window_size(&screen.height, &screen.width) == -1) die("init");

    // Hide cursor
    write(STDOUT_FILENO, "\x1b[?25l", 6); 
    clear_screen();
}

long get_ms() {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    return spec.tv_sec * 1000 + spec.tv_nsec / 1000000;
}

long get_us() {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    return spec.tv_sec * 1000000 + spec.tv_nsec / 1000;
}

long render(){
    long start = get_us();
    
    // Calculate center position
    int center_x = screen.width / 2;
    int center_y = screen.height / 2;
    
    screen.frame_buffer->c[center_x + center_y * (screen.width + 5) + 3] = '#';

    write(STDOUT_FILENO, screen.frame_buffer->c, screen.frame_buffer->used);
    screen.frame_buffer->used = 0;
    
    long end = get_us();
    char t_buf[32];
    long elapsed_ms = end - start;
    int len = snprintf(t_buf, sizeof(t_buf), "Time taken: %ld us\r", elapsed_ms);
    write(STDOUT_FILENO, t_buf, len);
    return elapsed_ms;
}

int main(){
    screen_init();
    enable_raw_mode();

    int times = 500;
    long total = 0;
    for(int i = 0; i < times; ++i){
        if (editor_read_key() == CTRL_KEY('q')){
            disable_raw_mode();
            return 0;
        }
        //render();
        total += render();
    }
    printf("Average Time taken: %ld us", total / times);
    buf_free(screen.frame_buffer);
}
