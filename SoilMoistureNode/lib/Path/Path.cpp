#include "Path.h"

#include <cstring>

namespace ZbW {
    const char *BasenameGet(const char *path) {
        const char *result = strrchr(path, '/');

        if (result == 0) {
            result = path;
        }
        else {
            /* Skip leading path delimiter */
            result = &result[1];
        }

        return result;
    }
}
