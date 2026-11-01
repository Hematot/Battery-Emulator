#ifndef __HW_TULEBO_CUSTOM_S3_ETH_H__
#define __HW_TULEBO_CUSTOM_S3_ETH_H__

#include "hal.h"

class TuleboCustomS3EthHal : public Esp32Hal {
 public:
  const char* name() { return "Tulebo Custom ESP32-S3 Dual-Zoe Ethernet"; }

  // 1. Växelriktaren (Sungrow via interna TWAI)
  virtual gpio_num_t CAN_TX_PIN() { return GPIO_NUM_33; }
  virtual gpio_num_t CAN_RX_PIN() { return GPIO_NUM_34; }

  // 2. SPI3 Buss-pinnar för dina två externa MCP2515-kort (Frånskilda från PSRAM)
  virtual gpio_num_t MCP2515_MOSI() { return GPIO_NUM_1; }
  virtual gpio_num_t MCP2515_MISO() { return GPIO_NUM_2; }
  virtual gpio_num_t MCP2515_SCK() { return GPIO_NUM_3; }

  // Zoe Batteri 1 - MCP2515 kontroller
  virtual gpio_num_t MCP2515_CS() { return GPIO_NUM_38; }
  virtual gpio_num_t MCP2515_INT() { return GPIO_NUM_39; }

  // Zoe Batteri 2 - Mappas på samma buss men med egna CS/INT
  virtual gpio_num_t MCP2517_CS2() { return GPIO_NUM_40; }
  virtual gpio_num_t MCP2517_INT2() { return GPIO_NUM_41; }
  
  // (Dela bussen fysiskt på kretskortet)
  virtual gpio_num_t MCP2517_SDI2() { return GPIO_NUM_1; } // MOSI
  virtual gpio_num_t MCP2517_SDO2() { return GPIO_NUM_2; } // MISO
  virtual gpio_num_t MCP2517_SCK2() { return GPIO_NUM_3; } // SCLK

  // 3. Reläer / Kontaktorer för Batteri 1 & 2
  virtual gpio_num_t POSITIVE_CONTACTOR_PIN() { return GPIO_NUM_42; }
  virtual gpio_num_t SECOND_BATTERY_CONTACTORS_PIN() { return GPIO_NUM_43; }

  // Standardknappen på kortet (GPIO 0)
  virtual gpio_num_t AP_BUTTON_PIN() { return GPIO_NUM_0; }

  std::vector<comm_interface> available_interfaces() {
    return { comm_interface::CanNative, comm_interface::CanAddonMcp2515 };
  }
};

#define HalClass TuleboCustomS3EthHal

#ifndef HW_CONFIGURED
#define HW_CONFIGURED
#else
#error Multiple HW defined! Please select a single HW
#endif

#endif // __HW_TULEBO_CUSTOM_S3_ETH_H__
