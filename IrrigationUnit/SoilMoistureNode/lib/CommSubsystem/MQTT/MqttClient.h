#pragma once

#include <fsm.h>
#include <StopWatch.h>
#include <WiFi/WiFiManager.h>

#include <string.h>
#include <unordered_map>

#include <PubSubClient.h>

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
 
namespace ZbW{
namespace CommSubsystem {
    #define MQTT_MESSAGE_HANDLER_NAME(name)     name ## Handler
    typedef void (*tMqttMessageHandler)(void *context, const char *topic, const void *data, size_t len);
    #define MQTT_MESSAGE_HANDLER_DECLARE(name)  void MQTT_MESSAGE_HANDLER_NAME(name)(void *context, const char *topic, const void *data, size_t len)

    typedef struct sMqttMessageHandlerRegistration {
        tMqttMessageHandler handler;
        void *              context;
    }tMqttMessageHandlerRegistration;

    class MqttClient {
        public:
            MqttClient(WiFiManager &manager, HardwareSerial &log = Serial);
            virtual ~MqttClient();

            void reset();

            int  run();

            void CredentialsSet(const char *user, const char *password);

            bool connect(const char *clientId, const char *broker, int port);
            bool connect(String& clientId, String& broker, int port);
            bool connected();
            bool disconnect();
            bool disconnected();

            bool publish(const char *topic, const String& data, bool retain = false);
            bool publish(const char *topic, const void *data, size_t len, bool retain = false);

            bool subscribe(const char *topic, tMqttMessageHandler handler, void *context);
            bool unsubscribe(const char *topic, tMqttMessageHandler handler, void *context);

        protected:
            virtual void OnTopicReceived(const char* topic, const void* data, size_t length);

        private:
            HardwareSerial &    m_log;
            WiFiManager &       m_wifi;
            PubSubClient        m_client;
            String              m_clientId;
            String              m_user;
            String              m_password;
            String              m_brokerIp;

        private:
            std::unordered_map<std::string, tMqttMessageHandlerRegistration>    m_topics;

        private:
            FSM *                   m_fsm;
            static const tFSM_State m_mqtt_states[];

            StopWatch               m_timeout;

            /* Connection States */
            static FSM_STATE_HANDLER(Unknown);
            static FSM_STATE_HANDLER(Disconnected);
            static FSM_STATE_HANDLER(Connecting);
            static FSM_STATE_HANDLER(Connected);
            static FSM_STATE_HANDLER(Disconnecting);                
            static FSM_STATE_HANDLER(ReconnectTimeout);                
    };
};
};