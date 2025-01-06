/* Terminal Renderer - A simple terminal graphics system */

/* Standard includes */
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

int times = 0;

/* Constants and macros */
#define true 1
#define false 0
#define CTRL_KEY(k) ((k) & 0x1f)

/* Special key codes */
enum SpecialKeys {
    ESCAPE_KEY = 27,
    RETURN_KEY = 13,  // Change to 10 for Linux
    BACKSPACE = 127,
    TIMEOUT_KEY = 0,
    LEFT_ARROW = 1000,
    RIGHT_ARROW,
    DOWN_ARROW,
    UP_ARROW,
    PAGE_UP,
    PAGE_DOWN
};

/* Data structures */
struct Buffer {
    char *c;
    int len;
    int used;
};

struct Screen {
    int width, height;
    struct termios og_termios;
    struct Buffer* f_frame_buf;
    struct Buffer* b_frame_buf;
    long start_time;
};


/* Global state */
struct Screen screen;


/* Buffer operations */
#define BUFFER_INIT {NULL, 0}

void buffer_append(struct Buffer *buf, char *str, int len) {
    memcpy(&buf->c[buf->used], str, len);
    buf->used += len;
}

void buf_free(struct Buffer *abuf) {
    free(abuf->c);
}

/* Terminal handling */
void die(const char *c) {
    write(STDOUT_FILENO, "\x1b[2J", 4);  // Clear screen
    write(STDOUT_FILENO, "\x1b[H", 3);   // Move cursor to top-left
    perror(c);
    exit(1);
}

void disable_raw_mode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &screen.og_termios) == -1)
        die("tcsetattr");
}

void enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &screen.og_termios) == -1)
        die("tcgetattr");
    atexit(disable_raw_mode);

    struct termios raw = screen.og_termios;
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;  // Read timeout in deciseconds

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

/* Input handling */
int editor_read_key() {
    int nread;
    char c = 0;
    while ((nread = read(STDIN_FILENO, &c, 1)) == -1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    
    if (c == ESCAPE_KEY) {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) == -1) return ESCAPE_KEY;
        if (read(STDIN_FILENO, &seq[1], 1) == -1) return ESCAPE_KEY;

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) == -1) return ESCAPE_KEY;
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                    }
                }
            }
            switch (seq[1]) {
                case 'A': return UP_ARROW;
                case 'B': return DOWN_ARROW;
                case 'C': return RIGHT_ARROW;
                case 'D': return LEFT_ARROW;
            }
        }
        return ESCAPE_KEY;
    }
    return c;
}

/* Window size handling */
int get_cursor_position(int *rows, int *cols) {
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

    return 0;
}

int get_window_size(int *rows, int *cols) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return get_cursor_position(rows, cols);
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

void swap_buf(){
    struct Buffer* temp = screen.f_frame_buf;
    screen.f_frame_buf = screen.b_frame_buf;
    screen.b_frame_buf = temp;
}

void clear_frame_buf(struct Buffer* buf){
    // For each line we need:
    // - screen.width characters
    // - \x1b[K\r\n (5 bytes) for clear line and newline
    int line_size = screen.width + 5;
    int total_size = 3 + (line_size * screen.height);  // 3 for initial \x1b[H
    
    memset(buf->c, ' ', total_size);
    memcpy(buf->c, "\x1b[H", 3);
    buf->len = total_size;

    // Copy the formatted line screen.height times
    for (int i = 0; i < screen.height; i++) {
        memcpy(&buf->c[3 + screen.width + i * line_size], "\x1b[K\r", 4);
    }

    buf->used = total_size;
}

/* Screen operations */
void clear_screen() {
    // For each line we need:
    // - screen.width characters
    // - \x1b[K\r\n (5 bytes) for clear line and newline
    int line_size = screen.width + 5;
    int total_size = 3 + (line_size * screen.height);  // 3 for initial \x1b[H
    
    memset(screen.f_frame_buf->c, ' ', total_size);
    memcpy(screen.f_frame_buf->c, "\x1b[H", 3);
    screen.f_frame_buf->len = total_size;

    // Copy the formatted line screen.height times
    for (int i = 0; i < screen.height; i++) {
        memcpy(&screen.f_frame_buf->c[3 + screen.width + i * line_size], "\x1b[K\r\n", 5);
    }

    screen.f_frame_buf->used = total_size;
}

/* Timing utilities */
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

void screen_init() {
    if (get_window_size(&screen.height, &screen.width) == -1) 
        die("get_window_size");

    // Hide cursor
    write(STDOUT_FILENO, "\x1b[?25l", 6); 
    
    // Initialize both buffers
    int line_size = screen.width + 5;
    int total_size = 3 + (line_size * screen.height);
    
    screen.f_frame_buf = malloc(sizeof(struct Buffer));
    screen.f_frame_buf->c = malloc(total_size);
    screen.f_frame_buf->len = total_size;
    screen.f_frame_buf->used = 0;
    
    screen.b_frame_buf = malloc(sizeof(struct Buffer));
    screen.b_frame_buf->c = malloc(total_size);
    screen.b_frame_buf->len = total_size;
    screen.b_frame_buf->used = 0;

    screen.start_time = get_ms();
    
    // Initial clear of both buffers
    clear_frame_buf(screen.f_frame_buf);
    clear_frame_buf(screen.b_frame_buf);
}

void*
thread_render(){
    // Calculate center position
    int center_x = screen.width / 2;
    int center_y = screen.height / 2;

    long current_time = get_ms();
    float elapsed = (current_time - screen.start_time) / 1000.0f;  // Convert to milliseconds
    float position = (elapsed * 0.1f) * screen.width;  // Adjust 0.1f to control speed
    
    // Draw character at center
    screen.f_frame_buf->c[times % screen.width + center_y * (screen.width + 5) + 3] = '#';
    //screen.b_frame_buf->c[(int)position % screen.width + center_y * (screen.width + 5) + 3] = '#';

    // Output frame buffer
    write(STDOUT_FILENO, screen.f_frame_buf->c, screen.f_frame_buf->used);
    return NULL;
}

void*
thread_clear(){
    clear_frame_buf(screen.b_frame_buf);
    return NULL;
}


/* Rendering */
long render() {
    long start = get_us();

    pthread_t threads[2];

    // Create threads
    pthread_create(&threads[0], NULL, thread_clear, NULL);
    pthread_create(&threads[1], NULL, thread_render, NULL);

    // Wait for threads to complete
    for(int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }

    swap_buf();

    // Calculate and display timing
    long end = get_us();
    long elapsed_ms = end - start;

    char t_buf[32];
    int len = snprintf(t_buf, sizeof(t_buf), "Time taken: %ld us\x1b[K\r", elapsed_ms);
    write(STDOUT_FILENO, t_buf, len);
    
    return elapsed_ms;
}

/* Main entry point */
int main() {
    screen_init();
    enable_raw_mode();

    long total = 0;
    long min_time = 1000000;  // 1 second in microseconds
    long max_time = 0;
    
    while (editor_read_key() != CTRL_KEY('q')) {
        long frame_time = render();
        total += frame_time;
        
        if (frame_time < min_time) min_time = frame_time;
        if (frame_time > max_time) max_time = frame_time;

        times += 2;
    }
    
    disable_raw_mode();
    printf("\nPerformance Stats:\n");
    printf("Average frame time: %ld us (%.2f FPS)\n", 
           total / times, 1000000.0 / (total / (float)times));
    printf("Best frame time: %ld us (%.2f FPS)\n", 
           min_time, 1000000.0 / min_time);
    printf("Worst frame time: %ld us (%.2f FPS)\n", 
           max_time, 1000000.0 / max_time);
           
    buf_free(screen.f_frame_buf);
    buf_free(screen.b_frame_buf);
    return 0;
}
