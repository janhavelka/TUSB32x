/// @file gpio.h
/// @brief Native GPIO setup recording; these range checks do not qualify a board.
#pragma once
#include <cstdint>
#include <esp_err.h>
using gpio_num_t = int;
#define GPIO_IS_VALID_GPIO(pin) ((pin) >= 0 && (pin) < 49)
#define GPIO_IS_VALID_OUTPUT_GPIO(pin) GPIO_IS_VALID_GPIO(pin)
enum gpio_mode_t { GPIO_MODE_INPUT = 1 };
enum gpio_pullup_t { GPIO_PULLUP_DISABLE = 0 };
enum gpio_pulldown_t { GPIO_PULLDOWN_DISABLE = 0 };
enum gpio_int_type_t { GPIO_INTR_DISABLE = 0 };
struct gpio_config_t {
  uint64_t pin_bit_mask;
  gpio_mode_t mode;
  gpio_pullup_t pull_up_en;
  gpio_pulldown_t pull_down_en;
  gpio_int_type_t intr_type;
};
esp_err_t gpio_config(const gpio_config_t* config);
int gpio_get_level(gpio_num_t pin);
esp_err_t gpio_reset_pin(gpio_num_t pin);
