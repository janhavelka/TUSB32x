/// @file main.cpp
/// @brief Two physical devices sharing one externally serialized bus and fair owner turns.
#include "../arduino/ExampleApp.h"
/// @brief Set up two explicit targets; an optional IRQ signals the secondary owner only.
void setup() { tusb32x_arduino::setupExample(true); }
/// @brief Alternate primary CLI and secondary service; each turn advances at most one owner.
void loop() { tusb32x_arduino::loopExample(); }
