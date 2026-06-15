#include "window.hpp"
#include <cmath>
#include <cstring>

// ---- vec3 ----------------------------------------------------------------

struct v3 {
    float x, y, z;
    v3 operator+(v3 b) const { return {x+b.x, y+b.y, z+b.z}; }
    v3 operator-(v3 b) const { return {x-b.x, y-b.y, z-b.z}; }
    v3 operator*(float t) const { return {x*t, y*t, z*t}; }
    v3 operator*(v3 b)  const { return {x*b.x, y*b.y, z*b.z}; }
    float dot(v3 b) const { return x*b.x + y*b.y + z*b.z; }
    float len()     const { return sqrtf(x*x + y*y + z*z); }
    v3 norm()       const { float l = len(); return {x/l, y/l, z/l}; }
};

static v3  cross(v3 a, v3 b) { return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x}; }
static v3  mix(v3 a, v3 b, float t) { return a*(1-t) + b*t; }
static float cf(float x) { return x < 0 ? 0 : (x > 1 ? 1 : x); }  // clamp to [0,1]

// ---- SDFs ----------------------------------------------------------------

static float sdSphere(v3 p, float r) { return p.len() - r; }

static float sdTorus(v3 p, float R, float r) {
    float q = sqrtf(p.x*p.x + p.z*p.z) - R;
    return sqrtf(q*q + p.y*p.y) - r;
}

static float sdPlane(v3 p, float h) { return p.y - h; }

// IQ smooth min
static float smin(float a, float b, float k) {
    float h = cf(0.5f + 0.5f*(b-a)/k);
    return a*h + b*(1-h) - k*h*(1-h);
}

// ---- scene ---------------------------------------------------------------

struct Hit { float d; int mat; }; // mat: 0=floor  1=sphere  2=torus

static Hit map(v3 p, float sa, float ca) {
    // torus rotated around Y (sa/ca precomputed per frame)
    v3 tp = {p.x*ca - p.z*sa, p.y, p.x*sa + p.z*ca};
    float dt = sdTorus(tp, 0.62f, 0.20f);

    // sphere bobbing inside
    float ds = sdSphere(p, 0.24f);

    // floor
    float df = sdPlane(p, -0.65f);

    float dm = smin(ds, dt, 0.14f);   // blob-merge sphere into torus

    if (dm < df) return {dm, ds < dt ? 1 : 2};
    return {df, 0};
}

static v3 calcNormal(v3 p, float sa, float ca) {
    const float e = 0.0005f;
    return v3{
        map({p.x+e,p.y,p.z},sa,ca).d - map({p.x-e,p.y,p.z},sa,ca).d,
        map({p.x,p.y+e,p.z},sa,ca).d - map({p.x,p.y-e,p.z},sa,ca).d,
        map({p.x,p.y,p.z+e},sa,ca).d - map({p.x,p.y,p.z-e},sa,ca).d
    }.norm();
}

static float hardShadow(v3 ro, v3 rd, float sa, float ca) {
    float t = 0.05f;
    for (int i = 0; i < 28; i++) {
        float h = map(ro + rd*t, sa, ca).d;
        if (h < 0.001f) return 0.0f;
        t += h;
        if (t > 4.5f) break;
    }
    return 1.0f;
}

static float ambOcc(v3 p, v3 n, float sa, float ca) {
    float occ = 0.0f, sca = 1.0f;
    for (int i = 0; i < 5; i++) {
        float h = 0.02f + 0.10f * float(i) / 4.0f;
        occ += (h - map(p + n*h, sa, ca).d) * sca;
        sca *= 0.78f;
    }
    return cf(1.0f - 2.8f * occ);
}

// ---- material & lighting -------------------------------------------------

static v3 matColor(int mat, v3 p) {
    if (mat == 0) {
        int cx = (int)floorf(p.x + 128.f) + (int)floorf(p.z + 128.f);
        float c = (cx & 1) ? 0.70f : 0.04f;
        return {c, c, c * 1.1f};
    }
    if (mat == 1) return {1.00f, 0.32f, 0.05f};  // orange sphere
    return {0.12f, 0.48f, 1.00f};                  // blue torus
}

static v3 render(v3 ro, v3 rd, float t, float sa, float ca) {
    const v3 sky = {0.01f, 0.01f, 0.05f};

    // march
    float dist = 0.01f;
    Hit h = {0.0f, -1};
    for (int i = 0; i < 80; i++) {
        h = map(ro + rd*dist, sa, ca);
        if (h.d < 0.001f || dist > 14.0f) break;
        dist += h.d;
    }

    if (dist > 14.0f || h.d > 0.01f)
        return sky;

    v3 p = ro + rd * dist;
    v3 n = calcNormal(p, sa, ca);

    // warm key light orbiting overhead
    float la = t * 0.65f + 0.8f;
    v3 lp  = {cosf(la)*2.4f, 1.9f, sinf(la)*2.4f};
    v3 ld  = (lp - p).norm();
    v3 hd  = (ld - rd).norm();   // blinn-phong half-dir

    float dif  = cf(n.dot(ld));
    float spe  = powf(cf(n.dot(hd)), 60.0f);
    float shd  = hardShadow(p + n*0.012f, ld, sa, ca);
    float occ  = ambOcc(p, n, sa, ca);

    v3 col = matColor(h.mat, p);

    // rim: bright edge glow from behind
    float rim  = powf(1.0f - cf(-(rd.dot(n))), 3.0f) * occ;

    v3 c = col *  0.04f * occ                          // ambient
         + col * v3{1.10f, 0.90f, 0.70f} * (dif*shd)  // diffuse
         + v3{1.20f, 1.10f, 0.95f}       * (spe*shd)  // specular
         + col * v3{0.06f, 0.12f, 0.40f} * rim;        // rim (cool fill)

    // depth fog
    float fog = 1.0f - expf(-dist * 0.075f);
    return mix(c, sky, fog);
}

static uint32_t toPixel(v3 c) {
    // gamma 2.2
    auto g = [](float x) -> uint32_t {
        return (uint32_t)(powf(cf(x), 1.0f/2.2f) * 255.0f + 0.5f);
    };
    return (0xFFu << 24) | (g(c.x) << 16) | (g(c.y) << 8) | g(c.z);
}

// ---- main ----------------------------------------------------------------

int main() {
    Window window;
    const int W = 128, H = 128;
    window.init(W, H, 4, "Raymarcher");
    auto* px = window.pixels();

    float t      = 0.0f;
    bool  paused = false;

    while (window.pollEvents()) {
        if (window.isKeyPressed(Key::Escape)) break;
        if (window.isKeyPressed(Key::Space))  paused = !paused;

        // precompute torus spin (half speed of camera orbit)
        float sa = sinf(t * 0.50f);
        float ca = cosf(t * 0.50f);

        // camera slowly orbits the scene
        float camA = t * 0.22f;
        v3 ro    = {cosf(camA)*2.7f, 0.90f, sinf(camA)*2.7f};
        v3 fwd   = (v3{0, 0, 0} - ro).norm();
        v3 right = cross(fwd, {0, 1, 0}).norm();
        v3 up    = cross(right, fwd);

        for (int y = 0; y < H; y++) {
            for (int x = 0; x < W; x++) {
                float u  =  (2.0f*x - W + 0.5f) / H;
                float v_ = -(2.0f*y - H + 0.5f) / H;
                v3 rd = (right*u + up*v_ + fwd*1.5f).norm();
                px[x + y*W] = toPixel(render(ro, rd, t, sa, ca));
            }
        }

        window.present();
        window.frameLimit(20);
        if (!paused) t += 1.0f / 20.0f;
    }

    return 0;
}
