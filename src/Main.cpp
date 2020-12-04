#include <Config.h>
#include <Gpio.h>
#include <Log.h>
#include <MqttPaho.h>
#include <NanoAkka.h>
#include <stdio.h>
#include <unistd.h>
Log logger(2048);
Thread mainThread("main");
MqttPaho mqtt(mainThread);
Gpio gpio6(6);
StaticJsonDocument<10240> jsonDoc;

int main(int argc, char** argv) {
  Sys::init();
  JsonObject mqttConfig = jsonDoc.as<JsonObject>();
  mqttConfig["connection"] = "tcp://test.mosquitto.org";
  mqtt.config(mqttConfig);
  mqtt.init();
  mqtt.connect();

  mqtt.fromTopic<int>("gpio6/value") >> gpio6.value;
  mqtt.fromTopic<std::string>("gpio6/mode") >> gpio6.mode;

  mainThread.run();
}