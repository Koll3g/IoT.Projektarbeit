#include "WiFiManager.h"

#include <byte2hex.h>

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
 
using namespace ZbW::CommSubsystem;

typedef enum eWiFiState {
    WIFI_STATE_UNKNOWN = 0,
    WIFI_STATE_DISCONNECTED,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_DISCONNECTING,

    WIFI_STATE_COUNT
}tWiFiState;

const tFSM_State WiFiManager::m_WiFiStates[] = {
    FSM_STATE_DESCRIBE("Unknown", WiFiManager::Unknown),
    FSM_STATE_DESCRIBE("Disconnected", WiFiManager::Disconnected),
    FSM_STATE_DESCRIBE("Connecting", WiFiManager::Connecting),
    FSM_STATE_DESCRIBE("Connected", WiFiManager::Connected),
    FSM_STATE_DESCRIBE("Disconnecting", WiFiManager::Disconnecting),

    FSM_STATE_LAST()
};

/**
 * @brief Construct a new wifi manager object
 * 
 * @param wifi  WiFi instance to be managed
 */
WiFiManager::WiFiManager(WiFiClass &wifi, HardwareSerial &log) : m_wifi(wifi), m_log(log), m_timeout(1000) {
    m_fsm = new FSM(WIFI_STATE_UNKNOWN, m_WiFiStates, this);   
    wifi.macAddress(m_mac);
}

WiFiManager::~WiFiManager() {
    delete m_fsm;
    m_fsm = 0;
}

void WiFiManager::reset() {
  while (!disconnected()) {
    run();
  }
}

int WiFiManager::run() {
    int result = -1;

    if (m_fsm) {
        result = m_fsm->run();
    }

    return result;
}

bool WiFiManager::connect(const char *ssid, const char *psk) {
    if (m_fsm->CurrentStateGet() == WIFI_STATE_DISCONNECTED) {
        m_ssid = ssid;
        m_psk = psk;

        m_fsm->NextStateSet(WIFI_STATE_CONNECTING);
        return true;
    }

    return false;
}

bool WiFiManager::connected() {
    return m_fsm->CurrentStateGet() == WIFI_STATE_CONNECTED;
}

void WiFiManager::disconnect() {
    uintptr_t state = m_fsm->CurrentStateGet();
    if ((state == WIFI_STATE_CONNECTED)
        || (state == WIFI_STATE_CONNECTING)
        || (state == WIFI_STATE_UNKNOWN)) {
        m_fsm->NextStateSet(WIFI_STATE_DISCONNECTING);
    }
}

bool WiFiManager::disconnected() {
    return m_fsm->CurrentStateGet() == WIFI_STATE_DISCONNECTED;
}

IPAddress WiFiManager::IpGet() {
    return m_wifi.localIP();
}

const uint8_t *WiFiManager::MacGet() {
    return m_mac;
}

FSM_STATE_HANDLER(WiFiManager::Unknown) {
    WiFiManager *me = (WiFiManager *)context;

    if (reason == FSM_REASON_ENTER) {
        me->m_fsm->NextStateSet(WIFI_STATE_DISCONNECTING);
    }
    return 0;
}

FSM_STATE_HANDLER(WiFiManager::Disconnected) {
    WiFiManager *me = (WiFiManager *)context;

    if (reason == FSM_REASON_ENTER) {
        me->m_log.println("WiFi disconnected.");
    }

    /* Do nothing and wait for outer event. */
    return 0;
}

FSM_STATE_HANDLER(WiFiManager::Connecting) {
    WiFiManager *me = (WiFiManager *)context;

    if (reason == FSM_REASON_ENTER) {
        me->m_log.print("Connecting to '");
        me->m_log.print(me->m_ssid);
        me->m_log.print("'...");
        me->m_wifi.begin(me->m_ssid, me->m_psk);
        me->m_timeout.start(500);
    }
    else if (reason == FSM_REASON_DO) {
        if (me->m_timeout.isTimeout()) {
            me->m_log.print(".");
            me->m_timeout.restart();
        }

        if (me->m_wifi.isConnected()) {
            me->m_fsm->NextStateSet(WIFI_STATE_CONNECTED);
        }
    }
    else if (reason == FSM_REASON_EXIT) {
        me->m_log.println("done");
    }
    return 0;
}

FSM_STATE_HANDLER(WiFiManager::Connected) {
    WiFiManager *me = (WiFiManager *)context;

    if (reason == FSM_REASON_ENTER) {
        me->m_log.println("WiFi connected");
        {
            me->m_log.print("\tIP address: ");
            me->m_log.println(me->m_wifi.localIP());
        }
        {
            char buffer[3];
            me->m_log.print("\tMAC address: ");
            for (int i = 0;i < ZBW_COMMSUBSYSTEM_WIFIMANAGER_MAC_LEN - 1;i++) {
                me->m_log.print(byte2hex(buffer, me->m_mac[i]));
                me->m_log.print(":");
            }
            me->m_log.println(byte2hex(buffer, me->m_mac[ZBW_COMMSUBSYSTEM_WIFIMANAGER_MAC_LEN - 1]));
        }
    }

    /* Do nothing and wait for outer event. */
    return 0;
}

FSM_STATE_HANDLER(WiFiManager::Disconnecting) {
    WiFiManager *me = (WiFiManager *)context;

    if (reason == FSM_REASON_ENTER) {
        me->m_log.print("WiFi disconnecting...");
        me->m_wifi.disconnect(true);
    }
    else if (reason == FSM_REASON_DO) {
        me->m_log.print(".");
        if (!me->m_wifi.isConnected()) {
            me->m_fsm->NextStateSet(WIFI_STATE_DISCONNECTED);
        }
    }
    else if (reason == FSM_REASON_EXIT) {
        me->m_log.println("done");
    }
    return 0;
}
