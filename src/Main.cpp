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
FILE* logFd = 0;

void myLogFunction(char* s, uint32_t length) {
  fprintf(logFd, "%s\n", s);
  fflush(logFd);
  fprintf(stdout, "%s\r\n", s);
}

void myLogInit(const char* logFile) {
  logFd = fopen(logFile, "a");
  if (logFd == NULL) {
    WARN("open logfile %s failed : %d %s", logFile, errno, strerror(errno));
  } else {
    logger.writer(myLogFunction);
  }
}
/*
+-----+-----+---------+------+---+---Pi 3B--+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 |     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
 |   2 |   8 |   SDA.1 |  OUT | 1 |  3 || 4  |   |      | 5v      |     |     |
 |   3 |   9 |   SCL.1 |   IN | 1 |  5 || 6  |   |      | 0v      |     |     |
 |   4 |   7 | GPIO. 7 |   IN | 1 |  7 || 8  | 0 | IN   | TxD     | 15  | 14  |
 |     |     |      0v |      |   |  9 || 10 | 1 | IN   | RxD     | 16  | 15  |
 |  17 |   0 | GPIO. 0 |   IN | 0 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
 |  27 |   2 | GPIO. 2 |  OUT | 1 | 13 || 14 |   |      | 0v      |     |     |
 |  22 |   3 | GPIO. 3 |  OUT | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
 |     |     |    3.3v |      |   | 17 || 18 | 0 | IN   | GPIO. 5 | 5   | 24  |
 |  10 |  12 |    MOSI |   IN | 0 | 19 || 20 |   |      | 0v      |     |     |
 |   9 |  13 |    MISO |   IN | 0 | 21 || 22 | 1 | IN   | GPIO. 6 | 6   | 25  |
 |  11 |  14 |    SCLK | ALT0 | 0 | 23 || 24 | 1 | OUT  | CE0     | 10  | 8   |
 |     |     |      0v |      |   | 25 || 26 | 1 | OUT  | CE1     | 11  | 7   |
 |   0 |  30 |   SDA.0 |   IN | 1 | 27 || 28 | 1 | IN   | SCL.0   | 31  | 1   |
 |   5 |  21 | GPIO.21 |   IN | 1 | 29 || 30 |   |      | 0v      |     |     |
 |   6 |  22 | GPIO.22 |  OUT | 0 | 31 || 32 | 0 | OUT  | GPIO.26 | 26  | 12  |
 |  13 |  23 | GPIO.23 |   IN | 0 | 33 || 34 |   |      | 0v      |     |     |
 |  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 0 | IN   | GPIO.27 | 27  | 16  |
 |  26 |  25 | GPIO.25 |  OUT | 1 | 37 || 38 | 0 | IN   | GPIO.28 | 28  | 20  |
 |     |     |      0v |      |   | 39 || 40 | 0 | IN   | GPIO.29 | 29  | 21  |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+---Pi 3B--+---+------+---------+-----+-----+
*/

// uint8_t gpioRaspberry[] = {4, 9, 10, 17, 18, 22, 23, 24, 25, 27}; //
// RAspberry Pi 1
// uint8_t gpioRaspberry[] = {0,1,2,3,4,5,6,7,21,22,23,24,25,26,27,28,29}; //
// wiringPi convention

int main(int argc, char** argv) {
  Sys::init();
  Gpio::init();
  if (loadConfig(jsonDoc, "gpio2mqtt.json")) {
    std::string jsonString;
    serializeJsonPretty(jsonDoc, jsonString);
    INFO(" config loaded : %s ", jsonString.c_str());
  }

  JsonObject config = jsonDoc["log"];
  std::string level = config["level"] | "I";
  logger.setLogLevel(level[0]);
  if (config["file"]) {
    std::string logFile = config["file"];
    myLogInit(logFile.c_str());
  }

  JsonObject mqttConfig = jsonDoc["mqtt"];
  mqtt.config(mqttConfig);
  mqtt.init();
  mqtt.connect();

  for (uint32_t i = 0; i < Gpio::raspberryGpio.size(); i++) {
    uint32_t gpioIdx = Gpio::raspberryGpio[i];
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
    } else {
      gpio->mode.on("INPUT");
    }
  }

  mainThread.run();
}
