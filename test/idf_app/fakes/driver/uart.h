/// @file uart.h
/// @brief Recording native UART subset, including nonblocking FIFO transmission.
#pragma once
#include <cstddef>
#include <cstdint>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
using uart_port_t = int;
constexpr uart_port_t UART_NUM_0 = 0;
constexpr int UART_PIN_NO_CHANGE = -1;
enum uart_word_length_t { UART_DATA_8_BITS = 3 };
enum uart_parity_t { UART_PARITY_DISABLE = 0 };
enum uart_stop_bits_t { UART_STOP_BITS_1 = 1 };
enum uart_hw_flowcontrol_t { UART_HW_FLOWCTRL_DISABLE = 0 };
enum uart_sclk_t { UART_SCLK_DEFAULT = 0 };
struct uart_config_t {
  int baud_rate;
  uart_word_length_t data_bits;
  uart_parity_t parity;
  uart_stop_bits_t stop_bits;
  uart_hw_flowcontrol_t flow_ctrl;
  uint8_t rx_flow_ctrl_thresh;
  uart_sclk_t source_clk;
  struct { uint32_t allow_pd : 1; } flags;
};
enum uart_event_type_t { UART_DATA, UART_BREAK, UART_BUFFER_FULL, UART_FIFO_OVF,
                         UART_FRAME_ERR, UART_PARITY_ERR, UART_DATA_BREAK,
                         UART_PATTERN_DET, UART_EVENT_MAX };
struct uart_event_t { uart_event_type_t type; size_t size; bool timeout_flag; };
bool uart_is_driver_installed(uart_port_t port);
esp_err_t uart_param_config(uart_port_t port, const uart_config_t* config);
esp_err_t uart_set_pin(uart_port_t port, int tx, int rx, int rts, int cts);
esp_err_t uart_driver_install(uart_port_t port, int rxBytes, int txBytes,
                              int queueSize, QueueHandle_t* queue, int flags);
esp_err_t uart_driver_delete(uart_port_t port);
int uart_read_bytes(uart_port_t port, void* data, uint32_t length, TickType_t ticks);
int uart_tx_chars(uart_port_t port, const char* data, uint32_t length);
esp_err_t uart_wait_tx_done(uart_port_t port, TickType_t ticks);
