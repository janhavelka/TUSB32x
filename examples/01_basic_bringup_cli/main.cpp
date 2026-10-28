/// @file main.cpp
/// @brief Interactive single-target bring-up using the shared CLI and explicit read-only startup.
#include "../arduino/ExampleApp.h"
/// @brief Application setup; diagnostic console remains available if qualification or startup fails.
void setup() { tusb32x_arduino::setupExample(false); }
/// @brief Bounded owner turn; USB backpressure never owns the I2C task.
void loop() { tusb32x_arduino::loopExample(); }
