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

struct Screen{
    int width, height;
    struct termios og_termios;
    int frames;
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

struct Buffer{
    char *c;
    int len;
    int used;
};

#define BUFFER_INIT {NULL, 0}

void ab_append(struct Buffer *abuf, char *str, int len){
    char *new = realloc(abuf->c, abuf->len + len);

    if (new == NULL) return;
    memcpy(&new[abuf->len], str, len);
    abuf->c = new;
    abuf->len += len;
}

void
buffer_append(struct Buffer *buf, char *str, int len){
    if(buf->c == NULL){
        buf->c = malloc(len);
    }
    memcpy(&buf->c[buf->used], str, len);
    buf->used += len;
}

void
buffer_write(struct Buffer *buf){
    buf->c = realloc(buf->c, buf->used);  // Save the reallocated pointer
    write(STDOUT_FILENO, buf->c, buf->used);
    buf->len = buf->used;  // Update len to match the new size
}

void ab_free(struct Buffer *abuf){
    free(abuf->c);
}

void
editor_refresh_screen(){
    struct Buffer ab = BUFFER_INIT;
    
    //hides cursor to prevent fickering while rendering
    ab_append(&ab, "\x1b[?25l", 6);
    //sets cursor on top left
    ab_append(&ab, "\x1b[H", 3);

    write(STDOUT_FILENO, ab.c, ab.len);

    ab_free(&ab);
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
screen_init(){
    struct Buffer buf = BUFFER_INIT;
    if (get_window_size(&screen.height, &screen.width) == -1) die("init");
    ab_append(&buf, "\x1b[?25l", 6);  // Hide cursor
    write(STDOUT_FILENO, buf.c, buf.len);
    ab_free(&buf);
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

void render(){
    long start = get_ms();

    struct Buffer buf = BUFFER_INIT;

    // Hide cursor at start of render
    ab_append(&buf, "\x1b[?25l", 6);
    
    // Clear screen
    ab_append(&buf, "\x1b[2J", 4);
    ab_append(&buf, "\x1b[H", 3);

    for(int i = 0; i < screen.height; ++i){
        ab_append(&buf, "\r\n", 2);
    }

    // Show cursor before final write
    ab_append(&buf, "\x1b[?25h", 6);

    write(STDOUT_FILENO, buf.c, buf.len);
    ab_free(&buf);
    long end = get_ms();

    char t_buf[32];
    long elapsed_ms = end - start;
    int len = snprintf(t_buf, sizeof(t_buf), "Time taken: %ld ms\r\n", elapsed_ms);
    write(STDOUT_FILENO, t_buf, len);
}

long render_2(){
    long start = get_us();
    int total_size = screen.width * screen.height * 2;  // rough estimate including escape sequences
    struct Buffer buf = {malloc(total_size), total_size, 0};
    
    // Calculate center position
    int center_x = screen.width / 2;
    int center_y = screen.height / 2;
    
    buffer_append(&buf, "\x1b[H", 3);  // Move to top
    
    char *line = malloc(screen.width + 6);  // +6 for escape sequence and \r\n
    
    // Pre-fill with spaces
    for(int j = 0; j < screen.width; j++) {
        line[j] = ' ';
    }
    
    // Add end-of-line sequences
    memcpy(line + screen.width, "\x1b[K\r\n", 5);
    
    // Render each line
    for(int i = 0; i < screen.height; ++i){
        if (i == center_y) {
            line[center_x] = '#';
        }
        buffer_append(&buf, line, screen.width + 5);
        if (i == center_y) {
            line[center_x] = ' ';  // Reset for next lines
        }
    }
    
    free(line);
    buffer_write(&buf);
    ab_free(&buf);
    
    long end = get_us();
    char t_buf[32];
    long elapsed_ms = end - start;
    int len = snprintf(t_buf, sizeof(t_buf), "Time taken: %ld us\r", elapsed_ms);
    write(STDOUT_FILENO, t_buf, len);
    return elapsed_ms;
}

long render_3(){
    long start = get_us();
    int total_size = screen.width * screen.height * 2;  // rough estimate including escape sequences
    struct Buffer buf = {malloc(total_size), total_size, 0};
    
    // Calculate center position
    int center_x = screen.width / 2;
    int center_y = screen.height / 2;
    
    ab_append(&buf, "\x1b[H", 3);  // Move to top
    
    char *line = malloc(screen.width + 6);  // +6 for escape sequence and \r\n
    
    // Pre-fill with spaces
    for(int j = 0; j < screen.width; j++) {
        line[j] = ' ';
    }
    
    // Add end-of-line sequences
    memcpy(line + screen.width, "\x1b[K\r\n", 5);
    
    // Render each line
    for(int i = 0; i < screen.height; ++i){
        if (i == center_y) {
            line[center_x] = '#';
        }
        ab_append(&buf, line, screen.width + 5);
        if (i == center_y) {
            line[center_x] = ' ';  // Reset for next lines
        }
    }
    
    free(line);
    write(STDOUT_FILENO, buf.c, buf.len);
    ab_free(&buf);
    
    long end = get_us();
    char t_buf[32];
    long elapsed_ms = end - start;
    int len = snprintf(t_buf, sizeof(t_buf), "Time taken: %ld us\r", elapsed_ms);
    write(STDOUT_FILENO, t_buf, len);
    return elapsed_ms;
}

int main(){
    screen.frames = 0;
    screen_init();
    enable_raw_mode();
    int times = 100;
    long total = 0;
    for(int i = 0; i < times; ++i){
        if (editor_read_key() == CTRL_KEY('q')){
            disable_raw_mode();
            return 0;
        }
        //render();
        total += render_3();
    }
    printf("Average Time taken: %ld us", total / times);
}
