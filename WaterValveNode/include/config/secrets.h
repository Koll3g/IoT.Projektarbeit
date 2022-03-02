#pragma once

/**
 * @file
 * @brief       Application Secrets Setup
 * 
 *              This file describes the interface to the application specific
 *              secrets. In order to create a local, custom configuration,
 *              see instructions in `src\config\README`.
 * 
 * @author      Ueli Niederer, ueli.niederer@niederer-engineering.ch         
 * @copyright   Copyright (c) 2021 by Niederer Engineering GmbH
 */
 
#ifdef __cplusplus
extern "C" {
#endif

    extern const char *SECRET_WIFI_SSID;
    extern const char *SECRET_WIFI_PASSWORD;

#ifdef __cplusplus
}
#endif
