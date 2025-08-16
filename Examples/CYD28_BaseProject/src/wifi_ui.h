#pragma once

#include <WiFi.h>

void wifi_ui_init();
void wifi_ui_scan();
int wifi_ui_get_network_count();
const char* wifi_ui_get_ssid(int index);
int wifi_ui_get_rssi(int index);
bool wifi_ui_get_encrypted(int index);
void wifi_ui_connect(const char* ssid, const char* password);
void wifi_ui_autoconnect();
bool wifi_ui_is_connected();
String wifi_ui_get_status();
void wifi_ui_save_credentials(const char* ssid, const char* password);
bool wifi_ui_load_credentials(const char* ssid, char* password, size_t max_len);

