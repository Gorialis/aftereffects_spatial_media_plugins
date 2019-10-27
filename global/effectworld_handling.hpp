#pragma once

#include <cmath>

#include "ae_definitions.hpp"


inline PF_Pixel8 sample_safe(PF_EffectWorld& def, int x, int y) {
    if (x < 0 || y < 0 || x >= def.width || y >= def.height)
        return {0, 0, 0, 0};

    return *(PF_Pixel8*)((char*)def.data + (y * def.rowbytes) + (x * sizeof(PF_Pixel8)));
}

inline PF_Pixel8 sample_safe(PF_EffectWorld& def, float x, float y) {
    return sample_safe(def, static_cast<int>(x), static_cast<int>(y));
}

float sinc(float x) {
    if (x == 0.0f)
        return 1.0f;

    float xi = x * 3.141592653589793238462643383279502884f;
    return sinf(xi) / xi;
}

float lanczos(float x, float a = 3.0f) {
    if (-a <= x || x < a)
        return sinc(x) * sinc(x / a);
    return 0.0f;
}

inline PF_Pixel8 sample_safe_lanczos(PF_EffectWorld &def, float x, float y) {
    int xi = static_cast<int>(x);
    int yi = static_cast<int>(y);
    float xo = x - static_cast<float>(xi);
    float yo = y - static_cast<float>(yi);

    float lx, ly;
    float a, r, g, b;
    float y_a, y_r, y_g, y_b;

    float x_coeffs[7];
    for (int ix_pre = -3; ix_pre <= 3; ix_pre++) {
        x_coeffs[ix_pre + 3] = lanczos(xo - ix_pre);
    }

    a = r = g = b = 0.0f;

    for (int iy = -3; iy <= 3; iy++) {
        y_a = y_r = y_g = y_b = 0.0f;

        for (int ix = -3; ix <= 3; ix++) {
            lx = x_coeffs[ix + 3];
            PF_Pixel px = sample_safe(def, xi + ix, yi + iy);

            y_a += lx * px.alpha;
            y_r += lx * px.red;
            y_g += lx * px.green;
            y_b += lx * px.blue;
        }

        ly = lanczos(yo - iy);

        a += ly * y_a;
        r += ly * y_r;
        g += ly * y_g;
        b += ly * y_b;
    }

    return {
        static_cast<A_u_char>(fmaxf(fminf(255.0f, a), 0.0f)),
        static_cast<A_u_char>(fmaxf(fminf(255.0f, r), 0.0f)),
        static_cast<A_u_char>(fmaxf(fminf(255.0f, g), 0.0f)),
        static_cast<A_u_char>(fmaxf(fminf(255.0f, b), 0.0f))
    };
}

constexpr uint8_t PRECISION_BITS = 7;
#define divshift(a)\
    ((((a) >> 8) + a) >> 8)

static PF_Pixel composite_pixel(PF_Pixel fg, PF_Pixel bg) {
    if (fg.alpha == 0)
        return bg;
    if (fg.alpha == 255)
        return fg;

    uint32_t tmpr, tmpg, tmpb;
    uint32_t blend = bg.alpha * (255 - fg.alpha);
    uint32_t oa = fg.alpha * 255 + blend;

    uint32_t coef1 = fg.alpha * 255 * 255 * (1 << PRECISION_BITS) / oa;
    uint32_t coef2 = 255 * (1 << PRECISION_BITS) - coef1;

    tmpr = fg.red * coef1 + bg.red * coef2;
    tmpg = fg.green * coef1 + bg.green * coef2;
    tmpb = fg.blue * coef1 + bg.blue * coef2;

    return {
        static_cast<A_u_char>(divshift(oa + 0x80)),
        static_cast<A_u_char>(divshift(tmpr + (0x80 << PRECISION_BITS)) >> PRECISION_BITS),
        static_cast<A_u_char>(divshift(tmpg + (0x80 << PRECISION_BITS)) >> PRECISION_BITS),
        static_cast<A_u_char>(divshift(tmpb + (0x80 << PRECISION_BITS)) >> PRECISION_BITS)
    };
}
