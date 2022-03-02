#pragma once

/**
 * @file
 * @brief       MQTT Setup
 * 
 *              This file describes the interface to the application specific
 *              MQTT settings. In order to create a local, custom configuration, 
 *              see instructions in `src\config\README`.
 * 
 * @author      Ueli Niederer, ueli.niederer@niederer-engineering.ch         
 * @copyright   Copyright (c) 2021 by Niederer Engineering GmbH
 */
 
#ifdef __cplusplus
extern "C" {
#endif

    extern const char *MQTT_CLIENT_ID;
    extern const char *MQTT_BROKER_IP;
    extern const int   MQTT_BROKER_PORT;

    extern const char *MQTT_PASSWORD;

#ifdef __cplusplus
}
#endif
