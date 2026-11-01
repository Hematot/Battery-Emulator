#pragma once

#include <Arduino.h> // Se till att basklasser som IPAddress alltid är kända

#ifndef CONFIG_W5500_ETHERNET
#include <WiFi.h>
#endif

// Shared network-state helpers
bool network_connected();
IPAddress network_localIP();

void network_bring_services_up(const IPAddress& ip);
