#include "network_status.h"

#ifdef CONFIG_W5500_ETHERNET
#include <ETH.h> // ESP32:s inbyggda, asynkrona LwIP-Ethernet
#else
#include <WiFi.h>
#include "../wifi/wifi.h"      // wifi_connected()
#endif

#include "../utils/logging.h"  // logging, syslog_start()
#include "mdns.h"              // init_mDNS()

bool network_connected() {
#ifdef CONFIG_W5500_ETHERNET
  return ETH.linkStatus() == LinkON && ETH.localIP() != IPAddress(0,0,0,0);
#else
  return wifi_connected();
#endif
}

IPAddress network_localIP() {
#ifdef CONFIG_W5500_ETHERNET
  return ETH.localIP();
#else
  return WiFi.localIP();
#endif
}

void network_bring_services_up(const IPAddress& ip) {
  LOG_SET_NEXT_SEVERITY(5);  // notice
  logging.printf("Got IP address: %s\n", ip.toString().c_str());
  syslog_start();  // safe to call more than once
#ifndef SMALL_FLASH_DEVICE
  init_mDNS();
#endif
}

#ifdef CONFIG_W5500_ETHERNET
// Händelselyssnare (Event Handler) som triggas asynkront av ESP32:s operativsystem
void on_eth_event(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("Ethernet har fått IP-adress: ");
      Serial.println(ETH.localIP());
      network_bring_services_up(ETH.localIP());
      break;
    case ARDUINO_EVENT_ETH_LOST_IP:
      Serial.println("Ethernet tappade IP-adressen.");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("Fysisk nätverkskabel ansluten.");
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("Fysisk nätverkskabel urdragen.");
      break;
    default:
      break;
  }
}

void init_waveshare_ethernet() {
  Serial.println("Registrerar Waveshare W5500 i ESP32:s nätverksstack...");

  int eth_cs_pin = 14; 
  int eth_rst_pin = 9;
  int eth_int_pin = 10; // W5500 interna avbrottspinne på Waveshare-kortet

  // Starta händelselyssnaren
  WiFi.onEvent(on_eth_event);

  // Gör en ren hårdvaru-reset på W5500-chippet innan start
  pinMode(eth_rst_pin, OUTPUT);
  digitalWrite(eth_rst_pin, LOW);
  delay(20);
  digitalWrite(eth_rst_pin, HIGH);
  delay(200);

  // Aktivera W5500 direkt i ESP32:s inbyggda LwIP-stack (Asynkront!)
  // Parametrar: (SPI_CS, SPI_INT, SPI_RST, SPI_CLK_SPEED, SPI_HOST)
  // På Waveshare sitter den på SPI2_HOST (FSPI)
  ETH.begin(ETH_PHY_W5500, 1, eth_cs_pin, eth_int_pin, eth_rst_pin, SPI2_HOST);
  
  Serial.println("Ethernet-initiering skickad till bakgrunden.");
}
#endif
