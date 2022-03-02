#include "LedUtils.h"

#include <cstdlib>
#include <cstring>

/**
 * @brief Like #strntorgb but the length is derived from the string passed
 * 
 * @copydoc #strntorgb
 */
bool strtorgb(tRGB *dst, const char *str) {
    return strntorgb(dst, str, strlen(str));
}

/**
 * @brief Like 
 * 
 * @param dst   Pointer to a #tRGB structure receiving the parsed color
 * @param str   Pointer to the string to be parsed
 * @param len   Length of the string to be parsed
 * 
 * @return  true    if the color was sucessfully parsed
 * @return  false   if the string was not parsable
 * 
 * @note In case the string could not be parsed, @p dst is set to #000000
 */
bool strntorgb(tRGB *dst, const char *str, size_t len) {
    bool result = false;
    
    dst->rgb = 0;
    
    if (str[0] == '#') {
        /* Create a local copy to ensure NUL-termination */
        char color_str[7];
        strncpy(color_str, &str[1], sizeof(color_str));
        color_str[6] = '\0';

        unsigned long rgb = strtoul(color_str, 0, 16);
        dst->comp.red = (rgb >> 16) & 0xFF;
        dst->comp.green = (rgb >>  8) & 0xFF;
        dst->comp.blue = (rgb >>  0) & 0xFF;
        result = true;
    }
    else if ((len == 3) 
             && (strncmp(str, "red", len) == 0)) {
        dst->comp.red = 255;
        result = true;
    }
    else if ((len == 5) 
             && (strncmp(str, "green", len) == 0)) {
        dst->comp.green = 255;
        result = true;
    }
    else if ((len == 4) 
             && (strncmp(str, "blue", len) == 0)) {
        dst->comp.blue = 255;
        result = true;
    }

    return result;
}
