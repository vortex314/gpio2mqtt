cmake_minimum_required(VERSION 3.0.0)
project(gpio2mqtt VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(gpio2mqtt  src/Main.cpp  src/Sys.cpp src/Timer.cpp   src/NanoAkka.cpp src/MqttPaho.cpp src/Gpio.cpp) 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

include_directories(src ../Common ../ArduinoJson/src ../paho.mqtt.c/src ../nanoAkka/main/ ../nanoAkka/components/wifi ../paho.mqtt.c/src)
target_link_libraries(gpio2mqtt -lpthread -L/home/lieven/workspace/paho.mqtt.c/build/output/ -l:libpaho-mqtt3c.a -L/home/lieven/workspace/Common/Debug/ -lCommon -lrt )
