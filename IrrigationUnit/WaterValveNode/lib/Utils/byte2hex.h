#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * @file
     * @brief       Simple byte 2 hex conversion
     * 
     * @author      Ueli Niederer, ueli.niederer@niederer-engineering.ch         
     * @copyright   Copyright (c) 2021 by Niederer Engineering GmbH
     */
    char * byte2hex(char buffer[3], uint8_t byte);

#ifdef __cplusplus
}
#endif
