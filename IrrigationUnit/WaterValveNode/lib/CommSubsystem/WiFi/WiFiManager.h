#pragma once

/**
 * @file
 * @brief       WiFi Manager
  * 
 *              Further simplification of the Arduino WiFi stack implementing
 *              a simple statemachine to deal with the WiFi status.
 * 
 * @author      Ueli Niederer, ueli.niederer@niederer-engineering.ch         
 * @copyright   Copyright (c) 2021 by Niederer Engineering GmbH
 */

#include <WiFi.h>
#include <HardwareSerial.h>
 
#include <fsm.h>
#include <StopWatch.h>

#define ZBW_COMMSUBSYSTEM_WIFIMANAGER_MAC_LEN   6

namespace ZbW{
namespace CommSubsystem {

    class WiFiManager {
        public: 
            WiFiManager(WiFiClass &wifi = WiFi, HardwareSerial &log = Serial);
            virtual ~WiFiManager();

            void reset();

            int run();

            bool connect(const char *ssid, const char *psk);
            bool connected();
            void disconnect(void);
            bool disconnected();

            IPAddress      IpGet();
            const uint8_t *MacGet();

            operator WiFiClient() { return m_client; }
            operator WiFiClient&() { return m_client; }

        private:
            WiFiClient      m_client;
            uint8_t         m_mac[ZBW_COMMSUBSYSTEM_WIFIMANAGER_MAC_LEN];

            const char *    m_ssid;
            const char *    m_psk;

            WiFiClass &         m_wifi;
            HardwareSerial &    m_log;

        private:
            StopWatch               m_timeout;

            FSM *                   m_fsm;
            static const tFSM_State m_WiFiStates[];

            /* Connection States */
            static FSM_STATE_HANDLER(Unknown);
            static FSM_STATE_HANDLER(Disconnected);
            static FSM_STATE_HANDLER(Connecting);
            static FSM_STATE_HANDLER(Connected);
            static FSM_STATE_HANDLER(Disconnecting);
    };
};
};