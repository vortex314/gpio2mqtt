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
/*
  load configuration file into JsonObject
*/
bool loadConfig(JsonDocument& doc, const char* name) {
  FILE* f = fopen(name, "rb");
  if (f == NULL) {
    ERROR(" cannot open config file : %s", name);
    return false;
  }
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  char* string = (char*)malloc(fsize + 1);
  fread(string, 1, fsize, f);
  fclose(f);

  string[fsize] = 0;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, string);
  if (error) {
    ERROR(" JSON parsing config file : %s failed.", name);
    return false;
  }
  return true;
}

// uint8_t gpioRaspberry[] = {4, 9, 10, 17, 18, 22, 23, 24, 25, 27}; //
// RAspberry Pi 1
uint8_t gpioRaspberry[] = {2, 4, 9, 10, 17, 18, 22, 23, 24, 25, 26, 27};

int main(int argc, char** argv) {
  Sys::init();
  Gpio::init();
  if (loadConfig(jsonDoc, "gpio2mqtt.json")) {
    std::string jsonString;
    serializeJsonPretty(jsonDoc, jsonString);
    INFO(" config loaded : %s ", jsonString.c_str());
  }

  JsonObject mqttConfig = jsonDoc["mqtt"];
  mqtt.config(mqttConfig);
  mqtt.init();
  mqtt.connect();

  for (uint32_t i = 0; i < sizeof(gpioRaspberry); i++) {
    uint32_t gpioIdx = gpioRaspberry[i];
    Gpio* gpio = new Gpio(mainThread, gpioIdx);
    std::string gpioValue = "gpio" + std::to_string(gpioIdx) + "/value";
    std::string gpioMode = "gpio" + std::to_string(gpioIdx) + "/mode";

    mqtt.fromTopic<int>(gpioValue.c_str()) >> gpio->value;
    mqtt.fromTopic<std::string>(gpioMode.c_str()) >> gpio->mode;
    gpio->value >> Cache<int>::nw(mainThread, 100, 1000) >>
        mqtt.toTopic<int>(gpioValue.c_str());
    gpio->mode >> Cache<std::string>::nw(mainThread, 100, 10000) >>
        mqtt.toTopic<std::string>(gpioMode.c_str());

    JsonArray gpioConfig = jsonDoc["gpio"][std::to_string(gpioIdx)];
    if (gpioConfig) {
      gpio->mode.on(gpioConfig[0]);
      gpio->value.on(gpioConfig[1]);
    }
  }

  mqtt.fromTopic<int>("src/pcdell/js0/axis0") >>
      mqtt.toTopic<int>("dst/GPIO/gpio25/value");

  mqtt.connected >> [](const bool& b) {
    if (b) mqtt.subscribe("src/pcdell/js0/axis0");
  };
  mainThread.run();
}
