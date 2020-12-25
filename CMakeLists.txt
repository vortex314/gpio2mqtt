cmake_minimum_required(VERSION 3.0.0)
project(gpio2mqtt VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(gpio2mqtt src/Main.cpp ) 

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(GCC_FLAGS "-DLINUX -g")
include(CPack)

add_definitions(${GCC_FLAGS})
include_directories(src  ../limero/inc ../limero/linux ../ArduinoJson/src ../paho.mqtt.c/src  )
target_link_libraries(gpio2mqtt -lpthread -L/home/lieven/workspace/limero/linux/build/ -llimero -L/home/lieven/workspace/paho.mqtt.c/build/output/ -l:libpaho-mqtt3c.a  -lrt -pthread )
