#include "network_status.h"

#ifdef CONFIG_W5500_ETHERNET
#include <SPI.h>
#include <Ethernet.h>
#else
#include <WiFi.h>
#include "../wifi/wifi.h"      // wifi_connected()
#endif

#include "../utils/logging.h"  // logging, syslog_start()
#include "mdns.h"              // init_mDNS()

#ifdef CONFIG_W5500_ETHERNET
bool ethernet_started = false;
#endif

bool network_connected() {
#ifdef CONFIG_W5500_ETHERNET
  if (!ethernet_started) return false;
  return Ethernet.linkStatus() == LinkON;
#else
  return wifi_connected();
#endif
}

IPAddress network_localIP() {
#ifdef CONFIG_W5500_ETHERNET
  return Ethernet.localIP();
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
void init_waveshare_ethernet() {
  if (ethernet_started) return;

  Serial.println("Initierar Waveshare W5500 Ethernet via SPI...");

  int eth_cs_pin = 14; 
  int eth_rst_pin = 9;

  // Gör en hårdvaru-reset på W5500-chippet
  pinMode(eth_rst_pin, OUTPUT);
  digitalWrite(eth_rst_pin, LOW);
  delay(20);
  digitalWrite(eth_rst_pin, HIGH);
  delay(200);

  // Säg till Ethernet-biblioteket vilken CS-pinne som gäller
  Ethernet.init(eth_cs_pin);

  // Hämta ESP32:ans inbyggda hårdvaru-MAC-adress så vi slipper hårdkoda en
  uint8_t mac[6];
  esp_efuse_read_mac_address_default(mac);

  // Starta Ethernet med ESP32:ans inbyggda MAC-adress via DHCP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Kritiskt fel: DHCP misslyckades för Ethernet!");
    ethernet_started = false;
  } else {
    ethernet_started = true;
    Serial.print("Ethernet startat! IP: ");
    Serial.println(Ethernet.localIP());
    network_bring_services_up(Ethernet.localIP());
  }
}
#endif
