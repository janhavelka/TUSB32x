/// @file ExampleApp.h
/// @brief Thin Arduino entry points sharing the portable CLI and application owners.
#pragma once
namespace tusb32x_arduino {
/// @brief Set up console and optional qualified, application-owned bus on the pinned loop core.
/// @param[in] twoDevices Enable the second, independently scheduled physical target.
/// @note Call once before loopExample on the pinned Arduino loop task. Resource creation and
/// checked identity-pointer preparation precede binding and are outside callback budgets.
void setupExample(bool twoDevices);
/// @brief One bounded input/owner/output turn; never waits for USB readiness or output space.
void loopExample();
/// @brief Signal shutdown from ordinary task context; no I2C or immediate resource destruction.
/// @note Not ISR-safe. The next loop stops admission, ends bindings, then releases owned Wire.
void requestStop();
} // namespace tusb32x_arduino
