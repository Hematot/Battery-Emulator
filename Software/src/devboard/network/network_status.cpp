#include "network_status.h"

#ifdef CONFIG_W5500_ETHERNET
#include <SPI.h>
#include <Ethernet.h>
#include <esp_mac.h>
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
  // Om vi redan är igång eller om vi väntar på DHCP, gör inget
  static unsigned long last_dhcp_attempt = 0;
  if (ethernet_started) return;

  // Begränsa hur ofta vi tjatar på routern om DHCP har misslyckats (t.ex. var 15:e sekund)
  if (last_dhcp_attempt > 0 && millis() - last_dhcp_attempt < 15000) {
    return;
  }
  
  last_dhcp_attempt = millis();
  Serial.println("Initierar Waveshare W5500 Ethernet via SPI...");

  int eth_cs_pin = 14; 
  int eth_rst_pin = 9;

  // Hårdvaru-reset på W5500
  pinMode(eth_rst_pin, OUTPUT);
  digitalWrite(eth_rst_pin, LOW);
  delay(20);
  digitalWrite(eth_rst_pin, HIGH);
  delay(200);

  Ethernet.init(eth_cs_pin);

  // Kolla om kabeln överhuvudtaget är i innan vi försöker med DHCP
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Nätverkskabel ej ansluten! Väntar...");
    ethernet_started = false;
    return; // Avbryt elegant. Loopen kommer att hoppa hit igen om 15 sekunder.
  }

  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  
  Serial.println("Begär IP-adress från router (DHCP)...");
  
  // Vi kör en kortare timeout (4 sekunder) så att vi inte blockerar processorn för länge
  if (Ethernet.begin(mac, 4000) == 0) {
    Serial.println("DHCP misslyckades. Försöker igen om 15 sekunder...");
    ethernet_started = false;
  } else {
    ethernet_started = true;
    Serial.print("Ethernet anslutet framgångsrikt! IP: ");
    Serial.println(Ethernet.localIP());
    network_bring_services_up(Ethernet.localIP());
  }
}
#endif

