#include <Gpio.h>

Gpio::Gpio(int pin) {
  _pin = pin;
  _mode = INPUT;
  mode >> [&](const std::string& m) {
    _mode = m[0] == 'O' ? OUTPUT : INPUT;
    INFO(" setting pin %d mode to %s ", _pin,
         _mode == INPUT ? "INPUT" : "OUTPUT");
  };
  value >> [&](const int& v) {
    _value = v ? 1 : 0;
    INFO(" setting pin %d value to %d ", _pin, _value);
  };
}

Gpio::~Gpio() {}