#pragma once

#include <stdint.h>
#include <stddef.h>

typedef union uRGB {
    struct sRGB {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    }comp;
    uint32_t rgb;
}tRGB;

bool strtorgb(tRGB *dst, const char *str);
bool strntorgb(tRGB *dst, const char *str, size_t len);