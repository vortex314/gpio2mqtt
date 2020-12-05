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
  JsonObject mqttConfig = jsonDoc.to<JsonObject>();
  mqttConfig["device"] = "GPIO";
  mqttConfig["connection"] = "tcp://limero.ddns.net";
  mqtt.config(mqttConfig);
  mqtt.init();
  mqtt.connect();

  gpio6.mode.on("OUTPUT");

  // mqtt.fromTopic<int>("gpio6/value") >> gpio6.value;

  mqtt.topic<int>("gpio6/value") == gpio6.value;

  mqtt.topic<std::string>("gpio6/mode") == gpio6.mode;

  mqtt.fromTopic<int>("src/pcdell/js0/axis0") >> 
      mqtt.toTopic<int>("dst/GPIO/gpio6/value");

  mqtt.connected >> [](const bool& b) {
    if (b) mqtt.subscribe("src/pcdell/js0/axis0");
  };
  mainThread.run();
}
