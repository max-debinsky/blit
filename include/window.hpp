#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class Window {
public:
    Window() = default;
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    ~Window();

    bool init(int logicalW, int logicalH, int scale, const char* title);
    bool pollEvents();
    void present();
    void frameLimit(int fps);
    void shutdown();

    uint32_t* pixels();

private:
    struct Impl {
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        SDL_Texture* texture = nullptr;
    };

    Impl* impl = nullptr;

    int width = 0;
    int height = 0;
    int scale = 1;

    std::chrono::time_point<std::chrono::high_resolution_clock> lastFrame;

    std::vector<uint32_t> framebuffer;
};