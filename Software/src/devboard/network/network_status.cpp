#include "network_status.h"
#include "../hal/hal.h" 

#ifdef CONFIG_W5500_ETHERNET
#include <SPI.h>
#include <ETH.h>
#include <Network.h> 
#endif

#include "../utils/logging.h"  
#include "mdns.h"              

#ifdef CONFIG_W5500_ETHERNET
void on_eth_event(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("[Ethernet] SUCCESS: IP-adress mottagen via DHCP: ");
      Serial.println(ETH.localIP());
      network_bring_services_up(ETH.localIP());
      break;
    case ARDUINO_EVENT_ETH_LOST_IP:
      Serial.println("[Ethernet] VARNING: Tappade IP-adressen.");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("[Ethernet] INFO: Fysisk länk etablerad (Kabel detekterad i uttaget).");
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("[Ethernet] VARNING: Fysisk nätverkskabel urdragen!");
      break;
    default:
      break;
  }
}
#endif

bool network_connected() {
#ifdef CONFIG_W5500_ETHERNET
  return ETH.linkUp() && ETH.localIP() != IPAddress(0,0,0,0);
#else
  extern bool wifi_connected();
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
  LOG_SET_NEXT_SEVERITY(5);  
  logging.printf("Got IP address: %s\n", ip.toString().c_str());
  syslog_start();  
#ifndef SMALL_FLASH_DEVICE
  init_mDNS();
#endif
}

#ifdef CONFIG_W5500_ETHERNET
void init_waveshare_ethernet() {
  // Vänta asynkront tills huvudtråden har initierat esp32hal ordentligt
  while (esp32hal == nullptr) {
    delay(50); 
  }

  Serial.println("[Ethernet] Startar initiering enligt beprövad Waveshare-syntax...");

  // Hämta pinnarna helt dynamiskt från din HAL-fil
  int eth_sck  = esp32hal->ETH_SCK_PIN();
  int eth_miso = esp32hal->ETH_MISO_PIN();
  int eth_mosi = esp32hal->ETH_MOSI_PIN();
  int eth_cs   = esp32hal->ETH_CS_PIN(); 
  int eth_rst  = esp32hal->ETH_RST_PIN();
  int eth_int  = esp32hal->ETH_INT_PIN(); 

  // Registrera händelselyssnaren för asynkrona meddelanden
  Network.onEvent(on_eth_event); 

  // 1. Konfigurera standard-SPI-pinnarna exakt som i ditt testprogram
  SPI.begin(eth_sck, eth_miso, eth_mosi);

  // 2. Starta nätverket med det inbyggda SPI-objektet (v3.x asynkron syntax)
  if (!ETH.begin(ETH_PHY_W5500, 1, eth_cs, eth_int, eth_rst, SPI)) {
    Serial.println("[Ethernet] Kritiskt fel: Kunde inte starta Ethernet-hårdvaran!");
  } else {
    Serial.println("[Ethernet] Initiering lyckades! DHCP-förhandling körs i bakgrunden...");
  }
}
#endif
