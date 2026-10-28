/// @file i2c_master.h
/// @brief Recording native resource and synchronous transfer APIs used by the real application.
#pragma once
#include <cstddef>
#include <cstdint>
#include <esp_err.h>
#include "gpio.h"
struct i2c_master_bus_t;
struct i2c_master_dev_t;
using i2c_master_bus_handle_t = i2c_master_bus_t*;
using i2c_master_dev_handle_t = i2c_master_dev_t*;
using i2c_port_num_t = int;
using i2c_clock_source_t = int;
constexpr int I2C_CLK_SRC_DEFAULT = 0;
constexpr int I2C_NUM_0 = 0;
constexpr uint16_t I2C_DEVICE_ADDRESS_NOT_USED = 0xFFFF;
enum i2c_addr_bit_len_t { I2C_ADDR_BIT_LEN_7 = 0, I2C_ADDR_BIT_LEN_10 = 1 };
struct i2c_master_bus_config_t {
  i2c_port_num_t i2c_port;
  gpio_num_t sda_io_num;
  gpio_num_t scl_io_num;
  i2c_clock_source_t clk_source;
  uint8_t glitch_ignore_cnt;
  int intr_priority;
  size_t trans_queue_depth;
  struct { uint32_t enable_internal_pullup : 1; uint32_t allow_pd : 1; } flags;
};
struct i2c_device_config_t {
  i2c_addr_bit_len_t dev_addr_length;
  uint16_t device_address;
  uint32_t scl_speed_hz;
  uint32_t scl_wait_us;
  struct { uint32_t disable_ack_check : 1; } flags;
};
esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t* config,
                             i2c_master_bus_handle_t* bus);
esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t bus,
    const i2c_device_config_t* config, i2c_master_dev_handle_t* device);
esp_err_t i2c_master_bus_rm_device(i2c_master_dev_handle_t device);
esp_err_t i2c_del_master_bus(i2c_master_bus_handle_t bus);
esp_err_t i2c_master_transmit(i2c_master_dev_handle_t device, const uint8_t* data,
                              size_t length, int timeoutMs);
esp_err_t i2c_master_receive(i2c_master_dev_handle_t device, uint8_t* data,
                             size_t length, int timeoutMs);
