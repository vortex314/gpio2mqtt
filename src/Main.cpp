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
Gpio gpio25(25);
StaticJsonDocument<10240> jsonDoc;

int main(int argc, char** argv) {
  Sys::init();
  Gpio::init();
  JsonObject mqttConfig = jsonDoc.to<JsonObject>();
  mqttConfig["device"] = "GPIO";
  mqttConfig["connection"] = "tcp://limero.ddns.net";
  mqtt.config(mqttConfig);
  mqtt.init();
  mqtt.connect();

  gpio25.mode.on("OUTPUT");

  // mqtt.fromTopic<int>("gpio25/value") >> gpio25.value;

  mqtt.topic<int>("gpio25/value") == gpio25.value;

  mqtt.topic<std::string>("gpio25/mode") == gpio25.mode;

  mqtt.fromTopic<int>("src/pcdell/js0/axis0") >>
      mqtt.toTopic<int>("dst/GPIO/gpio25/value");

  mqtt.connected >> [](const bool& b) {
    if (b) mqtt.subscribe("src/pcdell/js0/axis0");
  };
  mainThread.run();
}
