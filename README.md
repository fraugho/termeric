# Termeric

Termeric is a lightweight terminal graphics engine written in C that provides a simple interface for creating terminal-based graphical applications. It features multi-threaded rendering, performance monitoring, and buffer management.

## Features

- Multi-threaded architecture for improved performance
- Raw terminal mode handling for direct input control
- Performance monitoring and statistics
- Vector and buffer utilities for dynamic memory management
- Cross-platform terminal manipulation

## Requirements

- POSIX-compliant operating system
- C compiler with C11 support
- Terminal with ANSI escape sequence support

## Building

The project uses standard C headers and POSIX libraries.

To compile:
```bash
gcc main.c -o termeric
```

## Architecture

Termeric is composed of several modular components:

### Core Components

- `buffer.h`: Implements buffer management for terminal output
- `engine.h`: Main engine loop and thread management
- `screen.h`: Screen manipulation and drawing primitives
- `input.h`: Input handling and key mapping
- `timing.h`: High-precision timing utilities
- `vec.h`: Generic vector implementation for dynamic arrays

### Key Features

#### Threading Model
The engine uses two main threads:
- Render thread: Handles frame rendering
- I/O thread: Manages terminal output

#### Performance Monitoring
Built-in performance metrics include:
- Average frame time
- Best/worst frame times
- Real-time FPS calculation

#### Drawing Utilities
- Pixel drawing with custom characters
- Rectangle/square drawing
- Screen clearing and buffer management

## Usage

Basic example:

```c
#include "engine.h"
#include "input.h"

void render() {
    // Your rendering code here
    if(get_key() == CTRL_KEY('q')){
        engine_close();
    }
}

int main() {
    engine_init();
    return 0;
}
```

### Input Handling

The engine provides several pre-defined special keys:
- Arrow keys
- Page Up/Down
- Escape
- Control key combinations

### Screen Manipulation

Drawing operations available:
- `draw_pixel`: Place a character at specific coordinates
- `clean_square`: Clear a rectangular area
- `draw_blank`: Clear entire screen

## Performance

The engine includes built-in performance monitoring that tracks:
- Frame times
- FPS calculations
- Min/max frame times
- Average performance metrics

## Configuration

Several configurable parameters are available in `init.h`:
- `num_frames`: Number of frame buffers (default: 10)
- `BUF_SIZE`: Buffer size for screen output (default: 8000)
- `debug`: Enable/disable debug mode
