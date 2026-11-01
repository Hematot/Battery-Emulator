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
SPIClass ethSPI(SPI3_HOST); 

void on_eth_event(arduino_event_id_t event, arduino_event_info_t info) {
  // DEBUG 1: Se att operativsystemet faktiskt skickar nätverkshändelser överhuvudtaget!
  Serial.printf("[DEBUG ETH] on_eth_event triggad med Event-ID: %d\n", (int)event);

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
  // Kontrollera och vänta asynkront tills huvudtråden har initierat esp32hal ordentligt!
  // Detta förhindrar att vi kör return innan hårdvaruobjektet har skapats i minnet.
  while (esp32hal == nullptr) {
    delay(50); 
  }

  // Nu är esp32hal garanterat laddat och redo att läsas!
  Serial.println("[DEBUG ETH] STAG 1: init_waveshare_ethernet() har startat!");


  int eth_sck  = esp32hal->ETH_SCK_PIN();
  int eth_miso = esp32hal->ETH_MISO_PIN();
  int eth_mosi = esp32hal->ETH_MOSI_PIN();
  int eth_cs   = esp32hal->ETH_CS_PIN(); 
  int eth_rst  = esp32hal->ETH_RST_PIN();
  int eth_int  = esp32hal->ETH_INT_PIN(); 

  // DEBUG 3: Skriv ut exakt vilka pin-nummer mjukvaran har hämtat från din HAL-fil
  Serial.printf("[DEBUG ETH] STAG 2: Pinnar hämtade från HAL -> SCK:%d, MISO:%d, MOSI:%d, CS:%d, RST:%d, INT:%d\n", 
                eth_sck, eth_miso, eth_mosi, eth_cs, eth_rst, eth_int);

  Serial.println("[DEBUG ETH] STAG 3: Registrerar händelselyssnare...");
  Network.onEvent(on_eth_event); 

  Serial.println("[DEBUG ETH] STAG 4: Utför hårdvaru-reset på W5500...");
  pinMode(eth_rst, OUTPUT);
  digitalWrite(eth_rst, LOW);
  delay(50); // Något längre puls för att garantera att chippet hör oss
  digitalWrite(eth_rst, HIGH);
  delay(300);

  Serial.println("[DEBUG ETH] STAG 5: Startar SPI-bussen (SPI3_HOST)...");
  ethSPI.begin(eth_sck, eth_miso, eth_mosi, eth_cs);

  Serial.println("[DEBUG ETH] STAG 6: Skickar ETH.begin() till LwIP-nätverksstacken...");
  // Vi tvingar igång s3-specifik länkning bakom kulisserna
  ETH.begin(ETH_PHY_W5500, 1, eth_cs, eth_int, eth_rst, ethSPI);
  
  Serial.println("[DEBUG ETH] STAG 7: Initiering klar! Överlämnat till asynkron bakgrundstråd.");
}
#endif
