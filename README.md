# Pixel Display

A lightweight SDL2 abstraction layer for displaying raw pixel buffers.

## Features

- Simple window creation
- Pixel framebuffer presentation
- SDL2 abstraction
- Minimal dependencies
- Cross-platform

## Example

```cpp
Display display(640, 480, "Demo");

while (display.running())
{
    display.present(framebuffer);
}