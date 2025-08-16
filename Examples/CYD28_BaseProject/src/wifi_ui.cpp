#include "wifi_ui.h"
#include "CYD28_SD.h"
#include <SD.h>

#define WIFI_CREDS_FILE "/wifi_creds.txt"
#define MAX_NETWORKS 20

struct WiFiNetwork {
    String ssid;
    int32_t rssi;
    bool encrypted;
};

static WiFiNetwork networks[MAX_NETWORKS];
static int network_count = 0;
static bool wifi_scanning = false;

void wifi_ui_init() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
}

void wifi_ui_scan() {
    if (wifi_scanning) return;
    
    wifi_scanning = true;
    network_count = 0;
    
    Serial.println("Scanning for WiFi networks...");
    int n = WiFi.scanNetworks();
    
    if (n == 0) {
        Serial.println("No networks found");
    } else {
        Serial.printf("Found %d networks\n", n);
        
        // Sort by signal strength and remove duplicates
        for (int i = 0; i < n && network_count < MAX_NETWORKS; i++) {
            String ssid = WiFi.SSID(i);
            if (ssid.length() == 0) continue; // Skip hidden networks
            
            // Check for duplicates
            bool duplicate = false;
            for (int j = 0; j < network_count; j++) {
                if (networks[j].ssid == ssid) {
                    // Keep the one with stronger signal
                    if (WiFi.RSSI(i) > networks[j].rssi) {
                        networks[j].rssi = WiFi.RSSI(i);
                        networks[j].encrypted = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
                    }
                    duplicate = true;
                    break;
                }
            }
            
            if (!duplicate) {
                networks[network_count].ssid = ssid;
                networks[network_count].rssi = WiFi.RSSI(i);
                networks[network_count].encrypted = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
                network_count++;
            }
        }
        
        // Sort by signal strength (strongest first)
        for (int i = 0; i < network_count - 1; i++) {
            for (int j = i + 1; j < network_count; j++) {
                if (networks[i].rssi < networks[j].rssi) {
                    WiFiNetwork temp = networks[i];
                    networks[i] = networks[j];
                    networks[j] = temp;
                }
            }
        }
        
        Serial.printf("Processed %d unique networks\n", network_count);
    }
    
    wifi_scanning = false;
}

int wifi_ui_get_network_count() {
    return network_count;
}

const char* wifi_ui_get_ssid(int index) {
    if (index < 0 || index >= network_count) return "";
    return networks[index].ssid.c_str();
}

int wifi_ui_get_rssi(int index) {
    if (index < 0 || index >= network_count) return -100;
    return networks[index].rssi;
}

bool wifi_ui_get_encrypted(int index) {
    if (index < 0 || index >= network_count) return true;
    return networks[index].encrypted;
}

void wifi_ui_connect(const char* ssid, const char* password) {
    Serial.printf("Connecting to %s...\n", ssid);
    
    WiFi.disconnect();
    delay(100);
    
    if (strlen(password) > 0) {
        WiFi.begin(ssid, password);
    } else {
        WiFi.begin(ssid);
    }
    
    // Wait up to 15 seconds for connection
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        attempts++;
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
        wifi_ui_save_credentials(ssid, password);
    } else {
        Serial.println("\nConnection failed!");
    }
}

bool wifi_ui_is_connected() {
    return WiFi.status() == WL_CONNECTED;
}

String wifi_ui_get_status() {
    switch (WiFi.status()) {
        case WL_CONNECTED: return "Connected";
        case WL_CONNECT_FAILED: return "Connect Failed";
        case WL_CONNECTION_LOST: return "Connection Lost";
        case WL_DISCONNECTED: return "Disconnected";
        case WL_NO_SSID_AVAIL: return "No SSID Available";
        default: return "Unknown";
    }
}

void wifi_ui_save_credentials(const char* ssid, const char* password) {
    if (!SD.begin()) {
        Serial.println("SD card not available for saving credentials");
        return;
    }
    
    // Read existing credentials
    String credentials = "";
    bool found = false;
    
    if (SD.exists(WIFI_CREDS_FILE)) {
        File file = SD.open(WIFI_CREDS_FILE, FILE_READ);
        if (file) {
            while (file.available()) {
                String line = file.readStringUntil('\n');
                line.trim();
                if (line.length() > 0) {
                    int tabIndex = line.indexOf('\t');
                    if (tabIndex > 0) {
                        String stored_ssid = line.substring(0, tabIndex);
                        if (stored_ssid == ssid) {
                            // Update existing entry
                            credentials += String(ssid) + "\t" + String(password) + "\n";
                            found = true;
                        } else {
                            credentials += line + "\n";
                        }
                    }
                }
            }
            file.close();
        }
    }
    
    // Add new entry if not found
    if (!found) {
        credentials += String(ssid) + "\t" + String(password) + "\n";
    }
    
    // Write back to file
    File file = SD.open(WIFI_CREDS_FILE, FILE_WRITE);
    if (file) {
        file.print(credentials);
        file.close();
        Serial.printf("Saved credentials for %s\n", ssid);
    } else {
        Serial.println("Failed to save WiFi credentials");
    }
}

bool wifi_ui_load_credentials(const char* ssid, char* password, size_t max_len) {
    if (!SD.begin() || !SD.exists(WIFI_CREDS_FILE)) {
        return false;
    }
    
    File file = SD.open(WIFI_CREDS_FILE, FILE_READ);
    if (!file) return false;
    
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() > 0) {
            int tabIndex = line.indexOf('\t');
            if (tabIndex > 0) {
                String stored_ssid = line.substring(0, tabIndex);
                if (stored_ssid == ssid) {
                    String stored_password = line.substring(tabIndex + 1);
                    strncpy(password, stored_password.c_str(), max_len - 1);
                    password[max_len - 1] = '\0';
                    file.close();
                    return true;
                }
            }
        }
    }
    
    file.close();
    return false;
}

void wifi_ui_autoconnect() {
    if (!SD.begin() || !SD.exists(WIFI_CREDS_FILE)) {
        Serial.println("No saved WiFi credentials found");
        return;
    }
    
    Serial.println("Scanning for known networks...");
    wifi_ui_scan();
    
    File file = SD.open(WIFI_CREDS_FILE, FILE_READ);
    if (!file) return;
    
    String best_ssid = "";
    String best_password = "";
    int best_rssi = -100;
    
    // Find the best known network
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() > 0) {
            int tabIndex = line.indexOf('\t');
            if (tabIndex > 0) {
                String ssid = line.substring(0, tabIndex);
                String password = line.substring(tabIndex + 1);
                
                // Check if this network is available
                for (int i = 0; i < network_count; i++) {
                    if (networks[i].ssid == ssid && networks[i].rssi > best_rssi) {
                        best_ssid = ssid;
                        best_password = password;
                        best_rssi = networks[i].rssi;
                    }
                }
            }
        }
    }
    file.close();
    
    if (best_ssid.length() > 0) {
        Serial.printf("Auto-connecting to %s (RSSI: %d)\n", best_ssid.c_str(), best_rssi);
        wifi_ui_connect(best_ssid.c_str(), best_password.c_str());
    } else {
        Serial.println("No known networks found");
    }
}
