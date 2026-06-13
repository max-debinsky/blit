# Blit

A lightweight SDL2 abstraction layer for displaying raw pixel buffers.

## Features

- Simple window creation
- Pixel framebuffer presentation
- SDL2 abstraction
- Minimal dependencies
- Cross-platform

## Example

```cpp
int main() {
    Window window;
    window.init(128, 128, 4, "Demo Window");

    auto* px = window.pixels();  /// access to the pixel buffer

    while (window.pollEvents()) {
    
        window.present();

        window.frameLimit(60);
    }
}