#include "window.hpp"

#include <SDL2/SDL.h>
#include <chrono>
#include <cstring>
#include <thread>

static int toKeyIdx(SDL_Scancode sc) {
    if (sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z)
        return static_cast<int>(Key::A) + (sc - SDL_SCANCODE_A);
    if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_9)
        return static_cast<int>(Key::Num1) + (sc - SDL_SCANCODE_1);
    switch (sc) {
        case SDL_SCANCODE_0:         return static_cast<int>(Key::Num0);
        case SDL_SCANCODE_UP:        return static_cast<int>(Key::Up);
        case SDL_SCANCODE_DOWN:      return static_cast<int>(Key::Down);
        case SDL_SCANCODE_LEFT:      return static_cast<int>(Key::Left);
        case SDL_SCANCODE_RIGHT:     return static_cast<int>(Key::Right);
        case SDL_SCANCODE_SPACE:     return static_cast<int>(Key::Space);
        case SDL_SCANCODE_RETURN:    return static_cast<int>(Key::Enter);
        case SDL_SCANCODE_ESCAPE:    return static_cast<int>(Key::Escape);
        case SDL_SCANCODE_BACKSPACE: return static_cast<int>(Key::Backspace);
        case SDL_SCANCODE_TAB:       return static_cast<int>(Key::Tab);
        case SDL_SCANCODE_LSHIFT:    return static_cast<int>(Key::LShift);
        case SDL_SCANCODE_RSHIFT:    return static_cast<int>(Key::RShift);
        case SDL_SCANCODE_LCTRL:     return static_cast<int>(Key::LCtrl);
        case SDL_SCANCODE_RCTRL:     return static_cast<int>(Key::RCtrl);
        default:                     return -1;
    }
}

bool Window::init(int w, int h, int s, const char* title) {
    if (impl) return false;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return false;

    impl = new Impl();

    width = w;
    height = h;
    scale = s;

    framebuffer.resize(static_cast<size_t>(width) * height);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    impl->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width * scale,
        height * scale,
        SDL_WINDOW_SHOWN
    );

    if (!impl->window) {
        shutdown();
        return false;
    }

    impl->renderer = SDL_CreateRenderer(
        impl->window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (!impl->renderer) {
        shutdown();
        return false;
    }

    SDL_RenderSetIntegerScale(impl->renderer, SDL_TRUE);
    SDL_RenderSetLogicalSize(impl->renderer, width, height);

    impl->texture = SDL_CreateTexture(
        impl->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );

    if (!impl->texture) {
        shutdown();
        return false;
    }

    lastFrame = std::chrono::high_resolution_clock::now();
    return true;
}

uint32_t* Window::pixels() {
    return framebuffer.data();
}

bool Window::pollEvents() {
    memcpy(prevKeys,  currKeys,  sizeof(prevKeys));
    memcpy(prevMouse, currMouse, sizeof(prevMouse));

    bool running = true;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN: {
                int idx = toKeyIdx(event.key.keysym.scancode);
                if (idx >= 0) currKeys[idx] = true;
                break;
            }
            case SDL_KEYUP: {
                int idx = toKeyIdx(event.key.keysym.scancode);
                if (idx >= 0) currKeys[idx] = false;
                break;
            }

            case SDL_MOUSEMOTION:
                mx = event.motion.x;
                my = event.motion.y;
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                bool down = (event.type == SDL_MOUSEBUTTONDOWN);
                if (event.button.button == SDL_BUTTON_LEFT)   currMouse[0] = down;
                if (event.button.button == SDL_BUTTON_MIDDLE) currMouse[1] = down;
                if (event.button.button == SDL_BUTTON_RIGHT)  currMouse[2] = down;
                break;
            }
        }
    }

    return running;
}

void Window::present() {
    if (!impl) return;

    SDL_UpdateTexture(
        impl->texture,
        nullptr,
        framebuffer.data(),
        width * sizeof(uint32_t)
    );

    SDL_RenderClear(impl->renderer);
    SDL_RenderCopy(impl->renderer, impl->texture, nullptr, nullptr);
    SDL_RenderPresent(impl->renderer);
}

void Window::frameLimit(int fps) {
    auto frameTime = std::chrono::milliseconds(1000 / fps);
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = now - lastFrame;

    if (elapsed < frameTime)
        std::this_thread::sleep_for(frameTime - elapsed);

    lastFrame = std::chrono::high_resolution_clock::now();
}

void Window::shutdown() {
    if (!impl) return;

    if (impl->texture)  SDL_DestroyTexture(impl->texture);
    if (impl->renderer) SDL_DestroyRenderer(impl->renderer);
    if (impl->window)   SDL_DestroyWindow(impl->window);

    delete impl;
    impl = nullptr;

    framebuffer.clear();

    memset(currKeys,  0, sizeof(currKeys));
    memset(prevKeys,  0, sizeof(prevKeys));
    memset(currMouse, 0, sizeof(currMouse));
    memset(prevMouse, 0, sizeof(prevMouse));
    mx = my = 0;

    SDL_Quit();
}

Window::~Window() {
    shutdown();
}

// --- input queries ---

bool Window::isKeyDown(Key key) const {
    int idx = static_cast<int>(key);
    return idx >= 0 && idx < kKeyCount && currKeys[idx];
}

bool Window::isKeyPressed(Key key) const {
    int idx = static_cast<int>(key);
    return idx >= 0 && idx < kKeyCount && currKeys[idx] && !prevKeys[idx];
}

bool Window::isKeyReleased(Key key) const {
    int idx = static_cast<int>(key);
    return idx >= 0 && idx < kKeyCount && !currKeys[idx] && prevKeys[idx];
}

int Window::mouseX() const { return mx; }
int Window::mouseY() const { return my; }

bool Window::isMouseDown(MouseButton btn) const {
    return currMouse[static_cast<int>(btn)];
}

bool Window::isMousePressed(MouseButton btn) const {
    int i = static_cast<int>(btn);
    return currMouse[i] && !prevMouse[i];
}

bool Window::isMouseReleased(MouseButton btn) const {
    int i = static_cast<int>(btn);
    return !currMouse[i] && prevMouse[i];
}
