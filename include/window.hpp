#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

enum class Key {
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4,
    Num5, Num6, Num7, Num8, Num9,
    Up, Down, Left, Right,
    Space, Enter, Escape, Backspace, Tab,
    LShift, RShift, LCtrl, RCtrl,
    Count
};

enum class MouseButton { Left, Middle, Right };

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

    bool isKeyDown(Key key) const;
    bool isKeyPressed(Key key) const;
    bool isKeyReleased(Key key) const;

    int mouseX() const;
    int mouseY() const;
    bool isMouseDown(MouseButton btn) const;
    bool isMousePressed(MouseButton btn) const;
    bool isMouseReleased(MouseButton btn) const;

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

    static constexpr int kKeyCount = static_cast<int>(Key::Count);
    bool currKeys[kKeyCount] = {};
    bool prevKeys[kKeyCount] = {};

    int mx = 0, my = 0;
    bool currMouse[3] = {};
    bool prevMouse[3] = {};
};
