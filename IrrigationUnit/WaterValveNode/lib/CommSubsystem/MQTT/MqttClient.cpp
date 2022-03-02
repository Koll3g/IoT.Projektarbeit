#include "MqttClient.h"

#include <stdexcept>
#include <Arduino.h>

#include <byte2hex.h>

/**
 * @file
 * @brief       MQTT client
 * 
 *              Wrapper for the Arduino Pub-Sub-Client simplifying connection 
 *              handling as well as simplified topic/subscription handling.
 * 
 * @author      Ueli Niederer, ueli.niederer@niederer-engineering.ch         
 * @copyright   Copyright (c) 2021 by Niederer Engineering GmbH
 */

using namespace ZbW::CommSubsystem;
using namespace std;

typedef enum eMqttState {
    MQTT_STATE_UNKNOWN = 0,
    MQTT_STATE_DISCONNECTED,
    MQTT_STATE_CONNECTING,
    MQTT_STATE_CONNECTED,
    MQTT_STATE_DISCONNECTING,
    MQTT_STATE_RECONNECTTIMEOUT,

    MQTT_STATE_COUNT
}tMqttState;

const tFSM_State MqttClient::m_mqtt_states[] = {
    FSM_STATE_DESCRIBE("Unknown", MqttClient::Unknown),
    FSM_STATE_DESCRIBE("Disconnected", MqttClient::Disconnected),
    FSM_STATE_DESCRIBE("Connecting", MqttClient::Connecting),
    FSM_STATE_DESCRIBE("Connected", MqttClient::Connected),
    FSM_STATE_DESCRIBE("Disconnecting", MqttClient::Disconnecting),
    FSM_STATE_DESCRIBE("Reconnect Timeout", MqttClient::ReconnectTimeout),

    FSM_STATE_LAST()
};

const unsigned long CONNECT_TIMEOUT = 2000;
const unsigned long RECONNECT_WAIT_TIME = 5000;

MqttClient::MqttClient(WiFiManager &manager, HardwareSerial &log) : m_log(log), m_wifi(manager), m_client(manager), m_timeout(1000) {
    m_fsm = new FSM(MQTT_STATE_UNKNOWN, m_mqtt_states, this);

    m_client.setCallback([this](char *top, byte *pay, unsigned int len) {
        OnTopicReceived(top, pay, len); 
    });

    m_clientId = "";
}

MqttClient::~MqttClient() {
    if (m_client.connected()) {
        disconnect();
        while (m_client.connected()) {
            m_fsm->run();
        }
    }

    delete m_fsm;
    m_fsm = 0;
}

/**
 * @brief Initiates a connection to @p broker through @p port using the 
 *        given @p clientId
 * 
 * @param clientId  Client ID to be used when connecting to the broker
 * @param broker    Broker IP address to connect to.
 * @param port      Broker port to connect to.
 * 
 * @note  If the @p clientId passed is an empty string, the client ID will
 *        be generated with the following format: Node-<MAC Address>
 * 
 * @return true     If connection is initiated
 * @return false    If connection is already established
 */
bool MqttClient::connect(String& clientId, String& broker, int port) {
    if (m_client.connected()) {
        return false;
    }

    if (clientId == "") {
        char buffer[3];
        const uint8_t *mac = m_wifi.MacGet();
        clientId = "Node-";
        for (int i = 0;i < ZBW_COMMSUBSYSTEM_WIFIMANAGER_MAC_LEN;i++) {
            clientId += byte2hex(buffer, mac[i]);
        }
    }
    m_clientId = clientId;
    m_brokerIp = broker;
    m_client.setServer(m_brokerIp.c_str(), port);

    m_fsm->NextStateSet(MQTT_STATE_CONNECTING);
    return true;
}

/**
 * @copydoc  MqttClient::connect
 */
bool MqttClient::connect(const char *clientId, const char *broker, int port) {
    String clientIdStr(clientId);
    String brokerStr(broker);

    return connect(clientIdStr, brokerStr, port);
}

/**
 * @return true     if the client is connected
 * @return false    if the client is not connected (yet)
 */
bool MqttClient::connected() {
    return m_client.connected();
}

/**
 * @brief Triggers a disconnect of the client.
 * 
 * @return true     If the disconnect was triggered
 */
bool MqttClient::disconnect() {
    if (m_client.connected()) {
        m_fsm->NextStateSet(MQTT_STATE_DISCONNECTING);
        m_clientId = "";
    }
    return true;
}

/**
 * @return true if the client is disconnected
 * @return false if the client is currently connected
 */
bool MqttClient::disconnected() {
    return !m_client.connected();
}

/**
 * @brief Runs the internal logic for the MQTT client
 * 
 * @note Call this method on a regular basis, to keep the connection alive.
 * 
 * @return int 
 */
int MqttClient::run() {
    return m_fsm->run();
}

/**
 * @brief Resets the client by disconnecting it if it was already connected
 */
void MqttClient::reset() {
    while (!disconnected()) {
        run();
    }
}

/** 
 * @brief Set credentials to authenticate with
 */
void MqttClient::CredentialsSet(const char *user, const char *password) {
    m_user = user;
    m_password = password;
}

/**
 * @copydoc bool MqttClient::publish(const char *topic, const void *data, size_t len, bool retain)
 */
bool MqttClient::publish(const char *topic, const String& data, bool retain) {
    return publish(topic, data.c_str(), data.length());
}

/**
 * @brief Publish a @p topic with the payload specified through @p data 
 * 
 * @param topic     Topic to publish the payload to
 * @param data      Payload to be sent with the topic
 * @param retain    
 *                  - true if the topic shall be stored on the broker and not
 *                         only distributed
 *                  - false if the topic shall not be stored and only distributed
 *                          to the clients currently connected
 * 
 * @return true     If publishing the topic was successful
 * @return false    If the topic could not be published
 */
bool MqttClient::publish(const char *topic, const void *data, size_t len, bool retain) {
    return m_client.publish(topic, (const uint8_t *)data, len, retain);
}

/**
 * @brief   Subscribes the client to a given @p topic and registers a handler for the 
 *          given message.
 * 
 * @note    Currently wildcards are not supported by this client. Thus you have to 
 *          register for each exact topic you are interested in.
 * 
 * @param topic     Topic to subscribe for
 * @param handler   Handler to pass received topic and payload to
 * @param context   Context passed to the handler (e.g. "this")
 * 
 * @return true     If subscription was accepted
 * @return false    If subscription failed
 */
bool MqttClient::subscribe(const char *topic, tMqttMessageHandler handler, void *context) {
    tMqttMessageHandlerRegistration reg = {
        handler,
        context
    };
    unsubscribe(topic, handler, context);

    m_topics.insert(make_pair(string(topic), reg));
    m_client.subscribe(topic);
    return true;
}

/**
 * @brief   Unsubscribe the given @p handler for the specified @p topic 
 * 
 * @param topic     Topic to remove the handler from
 * @param handler   Handler to be removed from the topic
 * @param context   Context of the handler to be removed
 * 
 * @return true     If topic was successfully unsubscribed
 * @return false    If topic could not be unsubscribed
 */
bool MqttClient::unsubscribe(const char *topic, tMqttMessageHandler handler, void *context) {
    m_client.unsubscribe(topic);
    m_topics.erase(topic);
    return true;
}

/**
 * @brief   Topic reception handler
 *          
 *          This handler routes received topic values to the registered handlers
 * 
 * @param topic     Name of the topic received
 * @param data      The topic's payload
 * @param length    The payload's length
 */
void MqttClient::OnTopicReceived(const char *topic, const void* data, size_t length) {
    auto handlers = m_topics.find(string(topic));
    if (handlers != m_topics.end()) {
        handlers->second.handler(handlers->second.context, topic, data, length);
    }
}

FSM_STATE_HANDLER(MqttClient::Unknown) {
    MqttClient *me = (MqttClient *)context;

    if (reason == FSM_REASON_ENTER) {
        me->m_fsm->NextStateSet(MQTT_STATE_DISCONNECTING);
    }

    return 0;
}

FSM_STATE_HANDLER(MqttClient::Disconnected) {
    MqttClient *me = (MqttClient *)context;
    (void)me;

    /* Wait for external event. */
    return 0;
}

FSM_STATE_HANDLER(MqttClient::Connecting) {
    MqttClient *me = (MqttClient *)context;

    if (reason == FSM_REASON_ENTER) {
        if ((me->m_user.length() > 0)
            || (me->m_password.length() > 0)) {
            me->m_client.connect(me->m_clientId.c_str(), 
                                 me->m_user.c_str(), 
                                 me->m_password.c_str());
        }
        else {
            me->m_client.connect(me->m_clientId.c_str());
        }
        me->m_timeout.start(CONNECT_TIMEOUT);
    }
    else if (reason == FSM_REASON_DO) {
        int state = me->m_client.state();
        switch (state) {
            default:
            case MQTT_CONNECTION_TIMEOUT: 
            case MQTT_CONNECTION_LOST: 
            case MQTT_CONNECT_FAILED: 
            case MQTT_CONNECT_BAD_PROTOCOL: 
            case MQTT_CONNECT_BAD_CLIENT_ID: 
            case MQTT_CONNECT_UNAVAILABLE: 
            case MQTT_CONNECT_BAD_CREDENTIALS: 
            case MQTT_CONNECT_UNAUTHORIZED: 
            {
                me->m_log.print("Error while connecting. State: ");
                me->m_log.println(state);
                me->m_fsm->NextStateSet(MQTT_STATE_RECONNECTTIMEOUT);
                break;
            }

            case MQTT_DISCONNECTED:
            {
                /* Just wait... */
                break;
            }

            case MQTT_CONNECTED: 
            {
                /* Register all topics */
                for (auto i = me->m_topics.begin();i != me->m_topics.end();i++) {
                    me->m_client.subscribe(i->first.c_str());
                }
                me->m_fsm->NextStateSet(MQTT_STATE_CONNECTED);
                break;
            }
        }

        if (me->m_timeout.isTimeout()) {
            me->m_log.println("Connection attempt timed out.");
            me->m_client.disconnect();
            me->m_fsm->NextStateSet(MQTT_STATE_RECONNECTTIMEOUT);
        }
    }
    else if (reason == FSM_REASON_EXIT) {
    }

    return 0;
}

FSM_STATE_HANDLER(MqttClient::Connected) {
    MqttClient *me = (MqttClient *)context;

    if (reason == FSM_REASON_ENTER) {
    }
    else if (reason == FSM_REASON_DO) {
        if (!me->m_client.connected()) {
            me->m_fsm->NextStateSet(MQTT_STATE_RECONNECTTIMEOUT);
        }
        else {
            me->m_client.loop();
        }
    }
    else if (reason == FSM_REASON_EXIT) {
    }

    return 0;
}

FSM_STATE_HANDLER(MqttClient::Disconnecting) {
    MqttClient *me = (MqttClient *)context;

    if (reason == FSM_REASON_ENTER) {
        me->m_client.disconnect();
    }
    else if (reason == FSM_REASON_DO) {
        if (!me->m_client.connected()) {
            me->m_fsm->NextStateSet(MQTT_STATE_DISCONNECTED);
        }
    }

    return 0;
}

FSM_STATE_HANDLER(MqttClient::ReconnectTimeout) {
    MqttClient *me = (MqttClient *)context;

    if (reason == FSM_REASON_ENTER) {
        me->m_timeout.start(RECONNECT_WAIT_TIME);
        me->m_log.print("Lost connection, trying to reconnect in ");
        me->m_log.print(RECONNECT_WAIT_TIME);
        me->m_log.println("ms");
    }
    else if (reason == FSM_REASON_DO) {
        if (me->m_timeout.isTimeout()) {
            me->m_fsm->NextStateSet(MQTT_STATE_CONNECTING);
        }
    }

    return 0;
}
