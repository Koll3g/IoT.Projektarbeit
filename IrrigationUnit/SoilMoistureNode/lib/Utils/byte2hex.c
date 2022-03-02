#include "byte2hex.h"

/**
 * @file
 * @brief       Simple byte 2 hex conversion
 * 
 * @author      Ueli Niederer, ueli.niederer@niederer-engineering.ch         
 * @copyright   Copyright (c) 2021 by Niederer Engineering GmbH
 */

static const char HexDigits[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/**
 * @brief Converts the given byte to a three byte, 2 digit string.
 * 
 * @param buffer    Buffer to store the byte string in
 * @param byte      Byte to be converted
 * @return The buffer passed in through @p buffer for simplified use in call chains
 */
char * byte2hex(char buffer[3], uint8_t byte) {
    buffer[0] = HexDigits[(byte >> 4) & 0x0F];
    buffer[1] = HexDigits[(byte >> 0) & 0x0F];
    buffer[2] = 0;
    
    return buffer;
}