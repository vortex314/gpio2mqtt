#ifndef _MQTT_PAHO_H_
#define _MQTT_PAHO_H_
#include <Mqtt.h>
#include <NanoAkka.h>

#include "MQTTAsync.h"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#include <ArduinoJson.h>
#include <Mqtt.h>

#define QOS 0
#define TIMEOUT 10000L
#define MQTT_HOST "test.mosquitto.org"
#define MQTT_PORT 1883

class MqttPaho : public Mqtt {
 public:
  typedef enum {
    MS_CONNECTED,
    MS_DISCONNECTED,
    MS_CONNECTING
  } MqttConnectionState;
  typedef void (*StateChangeCallback)(void*, MqttConnectionState);
  typedef void (*OnMessageCallback)(void*, std::string, std::string);

 private:
  StaticJsonDocument<3000> _jsonBuffer;
  std::string _clientId;
  TimerSource _reportTimer;
  TimerSource _keepAliveTimer;
  std::string _hostPrefix;

  MQTTAsync_token _deliveredtoken;
  MQTTAsync _client;
  std::string _connection;
  int _keepAliveInterval;

  MqttConnectionState _connectionState;
  void state(MqttConnectionState newState);
  std::string _lastWillTopic;
  std::string _lastWillMessage;
  int _lastWillQos;
  bool _lastWillRetain;

  static void onConnectionLost(void* context, char* cause);
  static int onMessage(void* context, char* topicName, int topicLen,
                       MQTTAsync_message* message);
  static void onDisconnect(void* context, MQTTAsync_successData* response);
  static void onConnectFailure(void* context, MQTTAsync_failureData* response);
  static void onConnectSuccess(void* context, MQTTAsync_successData* response);
  static void onSubscribeSuccess(void* context,
                                 MQTTAsync_successData* response);
  static void onSubscribeFailure(void* context,
                                 MQTTAsync_failureData* response);
  static void onPublishSuccess(void* context, MQTTAsync_successData* response);
  static void onPublishFailure(void* context, MQTTAsync_failureData* response);
  static void onDeliveryComplete(void* context, MQTTAsync_token response);

 public:
  Sink<bool, 2> wifiConnected;
  ValueSource<bool> connected;
  TimerSource keepAliveTimer;
  MqttPaho(Thread& thread);
  ~MqttPaho();
  void init();
  void config(JsonObject&);
  int connection(std::string);
  int client(std::string);
  int connect();
  int disconnect();
  int publish(std::string topic, std::string message, int qos = 0,
              bool retain = false);
  //  int publish(std::string topic, Bytes message, int qos, bool retain);
  int subscribe(std::string topic);
  int lastWill(std::string topic, std::string message, int qos, bool retain);
  void onMessage(void* context, OnMessageCallback);
  MqttConnectionState state();

  void mqttPublish(const char* topic, const char* message);
  void mqttSubscribe(const char* topic);
  void mqttConnect();
  void mqttDisconnect();

  bool handleMqttMessage(const char* message);

  void onNext(const TimerMsg&);
  void onNext(const MqttMessage&);
  void request();
  /*  template <class T>
    Subscriber<T>& toTopic(const char* name) {
      auto flow = new ToMqtt<T>(name);
      *flow >> outgoing;
      return *flow;
    }
    template <class T>
    Source<T>& fromTopic(const char* name) {
      auto newSource = new FromMqtt<T>(name);
      incoming >> *newSource;
      return *newSource;
    }*/
  /*
                          template <class T>
                          MqttFlow<T>& topic(const char* name) {
                                  auto newFlow = new MqttFlow<T>(name);
                                  incoming >> newFlow->mqttIn;
                                  newFlow->mqttOut >> outgoing;
                                  return *newFlow;
                          }
                          void observeOn(Thread& thread);*/
};

//_______________________________________________________________________________________________________________
//

#endif
