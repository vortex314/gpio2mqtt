#include <Config.h>
#include <Gpio.h>
#include <Hardware.h>
#include <Log.h>
#include <MqttPaho.h>
#include <limero.h>
#include <stdio.h>
#include <unistd.h>
Log logger(2048);
Thread mainThread("main");
MqttPaho mqtt(mainThread);
StaticJsonDocument<10240> jsonDoc;

uint8_t gpioRaspberry[] = {4, 9, 10, 17, 18, 22, 23, 24, 25, 27};

int main(int argc, char** argv) {
  Sys::init();
  Gpio::init();

  JsonObject mqttConfig = jsonDoc.to<JsonObject>();
  mqttConfig["device"] = "GPIO";
  mqttConfig["connection"] = "tcp://limero.ddns.net";
  mqtt.config(mqttConfig);
  mqtt.init();
  mqtt.connect();

  for (uint32_t i = 0; i < sizeof(gpioRaspberry); i++) {
    uint32_t gpioIdx = gpioRaspberry[i];
    Gpio* gpio = new Gpio(mainThread, gpioIdx);
    gpio->mode.on("INPUT");
    std::string gpioValue = "gpio" + std::to_string(gpioIdx) + "/value";
    std::string gpioMode = "gpio" + std::to_string(gpioIdx) + "/mode";

    mqtt.fromTopic<int>(gpioValue.c_str()) >> gpio->value;
    mqtt.fromTopic<std::string>(gpioMode.c_str()) >> gpio->mode;
    gpio->value >> Cache<int>::nw(mainThread, 100, 1000) >>
        mqtt.toTopic<int>(gpioValue.c_str());
    gpio->mode >> Cache<std::string>::nw(mainThread, 100, 10000) >>
        mqtt.toTopic<std::string>(gpioMode.c_str());
    gpio->mode.on("INPUT");
    gpio->value.on(0);
    if (gpioIdx == 25) {
      gpio->mode.on("OUTPUT");
      gpio->value.on(1);
    }
  }

  mqtt.fromTopic<int>("src/pcdell/js0/axis0") >>
      mqtt.toTopic<int>("dst/GPIO/gpio25/value");

  mqtt.connected >> [](const bool& b) {
    if (b) mqtt.subscribe("src/pcdell/js0/axis0");
  };
  mainThread.run();
}
