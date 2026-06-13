#include "window.hpp"
#include <cmath>

struct vec2{
    float x,y;
};

struct vec3{
    float x,y,z;
};

vec2 project(vec3 vertex,
             float fov_degrees,
             int screen_width,
             int screen_height,
             float camera_z)
{
    float z = vertex.z + camera_z;

    if (z <= 0.01f)
        return {-1, -1}; // clipped

    float fov_radians = fov_degrees * (3.14159265f / 180.0f);
    float d = 1.0f / tan(fov_radians * 0.5f);

    float aspect = float(screen_width) / float(screen_height);

    float x_ndc = (d / aspect) * (vertex.x / z);
    float y_ndc = d * (vertex.y / z);

    return {
        ((x_ndc + 1.0f) * 0.5f) * screen_width,
        ((1.0f - y_ndc) * 0.5f) * screen_height
    };
}

void draw_pixel(vec2 pixel,
                uint32_t* px,
                int width,
                int height,
                uint32_t color)
{
    int x = (int)pixel.x;
    int y = (int)pixel.y;

    if (x < 0 || x >= width ||
        y < 0 || y >= height)
        return;

    px[x + y * width] = color;
}

void drawLine(vec2 a, vec2 b,
              uint32_t* px, int width, int height,
              uint32_t color)
{
    int x0 = (int)a.x;
    int y0 = (int)a.y;
    int x1 = (int)b.x;
    int y1 = (int)b.y;

    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;

    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;

    int err = dx + dy;

    while (true)
    {
        if (x0 >= 0 && x0 < width &&
            y0 >= 0 && y0 < height)
        {
            px[x0 + y0 * width] = color;
        }

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;

        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

vec3 vertices[8] = {
    { 1,  1,  1},
    {-1,  1,  1},
    { 1, -1,  1},
    {-1, -1,  1},
    { 1,  1, -1},
    {-1,  1, -1},
    { 1, -1, -1},
    {-1, -1, -1}
};

int edges[][2] = {
    {0,1}, {1,3}, {3,2}, {2,0}, // front face
    {4,5}, {5,7}, {7,6}, {6,4}, // back face
    {0,4}, {1,5}, {2,6}, {3,7}  // connecting edges
};

void rotateX(vec3* vertices, int count, float theta)
{
    float c = cos(theta);
    float s = sin(theta);

    for (int i = 0; i < count; i++)
    {
        float y = vertices[i].y;
        float z = vertices[i].z;

        vertices[i].y = y * c - z * s;
        vertices[i].z = y * s + z * c;
    }
}

void rotateY(vec3* vertices, int count, float theta)
{
    float c = cos(theta);
    float s = sin(theta);

    for (int i = 0; i < count; i++)
    {
        float x = vertices[i].x;
        float z = vertices[i].z;

        vertices[i].x = x * c + z * s;
        vertices[i].z = -x * s + z * c;
    }
}

void rotateZ(vec3* vertices, int count, float theta)
{
    float c = cos(theta);
    float s = sin(theta);

    for (int i = 0; i < count; i++)
    {
        float x = vertices[i].x;
        float y = vertices[i].y;

        vertices[i].x = x * c - y * s;
        vertices[i].y = x * s + y * c;
    }
}

int main() {
    Window window;

    const int height = 128;
    const int width = height;

    window.init(height, width, 4, "Demo Window");

    auto* px = window.pixels();

    rotateX(vertices, 8, 45);

    while (window.pollEvents()) {

        for(int i = 0; i < height*width; i++){
            px[i] = 0x0000000;
        }

        vec2 projected[8];
        for (int i = 0; i < 8; i++) {
            projected[i] = project(vertices[i], 40, height, width, 10);
        }

        for (int i = 0; i < 12; i++) {
            int a = edges[i][0];
            int b = edges[i][1];

            drawLine(
                {projected[a].x,
                projected[a].y},
                {projected[b].x,
                projected[b].y},
                px,
                height,
                width,
                0xFFFFFFFF
            );
        }

        //rotateX(vertices, 8, 0.01);
        rotateY(vertices, 8, 0.01);
        //rotateZ(vertices, 8, 0.01);

        window.present();

        window.frameLimit(60);
    }

    return 0;
}