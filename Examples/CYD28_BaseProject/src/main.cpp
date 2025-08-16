/**
 * @file main.cpp
 * @brief WiFi Manager for ESP32-2432S028 (CYD 2.8")
 */
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "CYD28_TouchscreenR.h"
#include "wifi_ui.h"

TFT_eSPI tft = TFT_eSPI();
CYD28_TouchR touch(320, 240);

// Global variables
int networkCount = 0;
int selectedNetwork = -1;
bool showPasswordInput = false;
String inputPassword = "";
String targetSSID = "";
unsigned long lastStatusUpdate = 0;
bool isScanning = false;
int scrollOffset = 0;
bool showPassword = false;
String debugMessage = "";

void displayNetworks();
void drawPasswordInput();

void debugPrint(String msg) {
	Serial.println(msg);
	debugMessage = msg;
	
	// Always show debug at very top, regardless of screen mode
	tft.fillRect(0, 0, 320, 12, TFT_BLACK);
	tft.setTextColor(TFT_YELLOW);
	tft.setTextFont(1);
	tft.setTextSize(1);
	tft.setCursor(2, 2);
	tft.print(msg);
}
void drawConnectionStatus();

void drawButton(int x, int y, int w, int h, uint16_t color, String text)
{
	tft.fillRect(x, y, w, h, color);
	tft.drawRect(x, y, w, h, TFT_WHITE);
	
	tft.setTextColor(TFT_BLACK);
	tft.setTextFont(1);
	int textX = x + (w - text.length() * 6) / 2;
	int textY = y + (h - 8) / 2;
	tft.setCursor(textX, textY);
	tft.print(text);
}

bool isButtonPressed(int touchX, int touchY, int btnX, int btnY, int btnW, int btnH)
{
	return (touchX >= btnX && touchX <= (btnX + btnW) && 
			touchY >= btnY && touchY <= (btnY + btnH));
}

void setup()
{
	Serial.begin(115200);
	debugPrint("WiFi Manager Starting...");

	// Turn on backlight
	pinMode(21, OUTPUT);
	digitalWrite(21, HIGH);
	delay(100);

	// Initialize display and touch
	tft.init();
	tft.setRotation(3);
	touch.begin();
	touch.setRotation(3);

	// Clear screen
	tft.fillScreen(TFT_BLACK);

	// Draw title
	tft.setTextColor(TFT_WHITE);
	tft.setTextFont(2);
	tft.setCursor(10, 15);
	tft.print("WiFi Manager");

	// Initialize WiFi UI
	wifi_ui_init();

	// Try auto-connect on startup
	wifi_ui_autoconnect();

	// Draw SCAN button
	drawButton(240, 40, 70, 30, TFT_GREEN, "SCAN");
	
	// Draw CONNECT button
	drawButton(10, 200, 100, 30, TFT_ORANGE, "CONNECT");
	
	// Draw DISCONNECT button (initially hidden)
	if (WiFi.status() == WL_CONNECTED) {
		drawButton(120, 200, 100, 30, TFT_RED, "DISCONNECT");
	}
	
	// Show initial connection status
	drawConnectionStatus();

	debugPrint("Setup complete. Ready for WiFi management!");
}

void loop()
{
	if (showPasswordInput) {
		// Handle password input interface
		if (touch.touched()) {
			CYD28_TS_Point p = touch.getPointScaled();
			
			// Check checkbox for show/hide password
			if (p.x >= 255 && p.x <= 275 && p.y >= 85 && p.y <= 105) {
				showPassword = !showPassword;
				drawPasswordInput();
			}
			// Check keyboard area
			else {
				// Virtual keyboard handling
				String keys[4][12] = {
					{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "@", "."},
					{"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "-", "_"},
					{"a", "s", "d", "f", "g", "h", "j", "k", "l", "#", "&", "DEL"},
					{"z", "x", "c", "v", "b", "n", "m", "!", "SPC", "OK", "CANCEL", ""}
				};
				
				int keyWidth = 25;
				int keyHeight = 25;
				int startX = 5;
				int startY = 135;
				
				for (int row = 0; row < 4; row++) {
					for (int col = 0; col < 12; col++) {
						if (keys[row][col] == "") continue;
						
						int kx = startX + col * (keyWidth + 2);
						int ky = startY + row * (keyHeight + 3);
						int kw = keyWidth;
						
						// Adjust width for special buttons
						if (keys[row][col] == "OK") kw = keyWidth + 10;
						else if (keys[row][col] == "CANCEL") kw = keyWidth + 15;
						else if (keys[row][col] == "DEL") kw = keyWidth + 5;
						else if (keys[row][col] == "SPC") kw = keyWidth + 5;					if (isButtonPressed(p.x, p.y, kx, ky, kw, keyHeight)) {
						if (keys[row][col] == "OK") {
							// Connect with password
							debugPrint("Connecting to " + targetSSID + " with password");
							
							// Show connecting status
							tft.fillRect(120, 200, 190, 30, TFT_BLACK);
							tft.setTextColor(TFT_YELLOW);
							tft.setTextFont(1);
							tft.setCursor(120, 210);
							tft.print("Connecting...");
							
							wifi_ui_connect(targetSSID.c_str(), inputPassword.c_str());
							
							// Return to main screen
							showPasswordInput = false;
							inputPassword = "";
							targetSSID = "";
							
							// Redraw main screen
							tft.fillScreen(TFT_BLACK);
							tft.setTextColor(TFT_WHITE);
							tft.setTextSize(2);
							tft.setCursor(40, 15);
							tft.println("WiFi Manager");
							
							// Reset text size
							tft.setTextSize(1);
							
							// Redraw buttons
							drawButton(240, 40, 70, 30, TFT_GREEN, "SCAN");
							drawButton(10, 200, 100, 30, TFT_ORANGE, "CONNECT");
							
							displayNetworks();
							drawConnectionStatus();
							
						} else if (keys[row][col] == "CANCEL") {
							// Cancel and return to main screen
							showPasswordInput = false;
							inputPassword = "";
							targetSSID = "";
							
							// Redraw main screen
							tft.fillScreen(TFT_BLACK);
							tft.setTextColor(TFT_WHITE);
							tft.setTextSize(2);
							tft.setCursor(40, 15);
							tft.println("WiFi Manager");
							
							// Reset text size
							tft.setTextSize(1);
							
							// Redraw buttons
							drawButton(240, 40, 70, 30, TFT_GREEN, "SCAN");
							drawButton(10, 200, 100, 30, TFT_ORANGE, "CONNECT");
							
							displayNetworks();
							drawConnectionStatus();
							
						} else if (keys[row][col] == "DEL") {
							// Delete last character
							if (inputPassword.length() > 0) {
								inputPassword = inputPassword.substring(0, inputPassword.length() - 1);
								drawPasswordInput();
							}
						} else if (keys[row][col] == "SPC") {
							// Add space
							inputPassword += " ";
							drawPasswordInput();
						} else {
							// Add character
							inputPassword += keys[row][col];
							drawPasswordInput();
						}
						
						// Wait for release
						while (touch.touched()) {
							delay(10);
						}
						delay(100);
						break;
					}
				}
			}
		}
	}
	} else {
		// Normal WiFi manager interface
		if (touch.touched())
		{
			CYD28_TS_Point p = touch.getPointScaled();
			
			// SCAN button
			if (isButtonPressed(p.x, p.y, 240, 40, 70, 30)) {
				if (!isScanning) {
					debugPrint("SCAN button pressed!");
					isScanning = true;
					drawButton(240, 40, 70, 30, TFT_DARKGREY, "SCAN");
					wifi_ui_scan();
					selectedNetwork = -1;
					scrollOffset = 0;
					displayNetworks();
					isScanning = false;
					drawButton(240, 40, 70, 30, TFT_GREEN, "SCAN");
				}
			}
			
			// Network selection and scrolling
			else if (p.x >= 10 && p.x <= 290 && p.y >= 80 && p.y <= 200) {
				int networkIndex = ((p.y - 80) / 20) + scrollOffset;
				if (networkIndex >= 0 && networkIndex < wifi_ui_get_network_count()) {
					selectedNetwork = networkIndex;
					debugPrint("Selected network: " + String(wifi_ui_get_ssid(selectedNetwork)));
					displayNetworks();
				}
			}
			// Scrollbar area
			else if (p.x >= 295 && p.x <= 300 && p.y >= 80 && p.y <= 200) {
				int maxScroll = max(0, wifi_ui_get_network_count() - 6);
				scrollOffset = ((p.y - 80) * maxScroll) / 120;
				if (scrollOffset < 0) scrollOffset = 0;
				if (scrollOffset > maxScroll) scrollOffset = maxScroll;
				displayNetworks();
			}
			
			// CONNECT button
			else if (isButtonPressed(p.x, p.y, 10, 200, 100, 30)) {
				if (selectedNetwork >= 0) {
					debugPrint("CONNECT button pressed!");
					const char* ssid = wifi_ui_get_ssid(selectedNetwork);
					
					if (wifi_ui_get_encrypted(selectedNetwork)) {
						// Show password input
						targetSSID = String(ssid);
						inputPassword = "";
						showPasswordInput = true;
						drawPasswordInput();
					} else {
						// Connect without password
						tft.fillRect(225, 200, 95, 30, TFT_BLACK);
						tft.setTextColor(TFT_YELLOW);
						tft.setTextFont(1);
						tft.setCursor(225, 210);
						tft.print("Connecting...");
						
						wifi_ui_connect(ssid, "");
					}
				}
			}
			// DISCONNECT button
			else if (isButtonPressed(p.x, p.y, 120, 200, 100, 30)) {
				debugPrint("DISCONNECT button pressed!");
				WiFi.disconnect();
				drawConnectionStatus();
			}

			// Wait for release
			while (touch.touched()) {
				delay(10);
			}
			delay(100);
		}
	}
	
	// Update connection status every 5 seconds
	if (millis() - lastStatusUpdate > 5000) {
		if (!showPasswordInput) {
			drawConnectionStatus();
		}
		lastStatusUpdate = millis();
	}
	
	delay(20);
}

void displayNetworks() {
	// Clear network area
	tft.fillRect(10, 80, 300, 120, TFT_BLACK);
	
	int count = wifi_ui_get_network_count();
	tft.setTextFont(1);
	tft.setTextSize(1);  // Reset text size
	
	// Show network count
	tft.setTextColor(TFT_CYAN);
	tft.setCursor(10, 65);
	tft.print("Found: ");
	tft.print(count);
	tft.print(" networks");
	
	int visibleCount = min(count - scrollOffset, 6);
	
	for (int i = 0; i < visibleCount; i++) {
		int networkIndex = i + scrollOffset;
		int y = 80 + (i * 20);  // Bigger spacing (20 instead of 15)
		
		// Highlight selected network
		if (networkIndex == selectedNetwork) {
			tft.fillRect(10, y-2, 280, 18, TFT_BLUE);
			tft.setTextColor(TFT_WHITE);
		} else {
			tft.setTextColor(TFT_WHITE);
		}
		
		tft.setCursor(15, y);
		
		// Show lock icon for encrypted networks
		if (wifi_ui_get_encrypted(networkIndex)) {
			tft.print("[L] ");
		} else {
			tft.print("[ ] ");
		}
		
		// Show SSID
		tft.print(wifi_ui_get_ssid(networkIndex));
		
		// Show signal strength
		int rssi = wifi_ui_get_rssi(networkIndex);
		tft.setCursor(250, y);
		if (rssi > -50) tft.print("****");
		else if (rssi > -60) tft.print("*** ");
		else if (rssi > -70) tft.print("**  ");
		else tft.print("*   ");
	}
	
	// Draw scrollbar if needed
	if (count > 6) {
		int scrollBarHeight = 120;
		int thumbHeight = (6 * scrollBarHeight) / count;
		int thumbPos = (scrollOffset * scrollBarHeight) / count;
		
		// Scrollbar track
		tft.drawRect(295, 80, 5, scrollBarHeight, TFT_DARKGREY);
		// Scrollbar thumb
		tft.fillRect(296, 80 + thumbPos, 3, thumbHeight, TFT_WHITE);
	}
}

void drawPasswordInput() {
	// Clear screen
	tft.fillScreen(TFT_BLACK);
	
	// Title
	tft.setTextColor(TFT_WHITE);
	tft.setTextSize(2);
	tft.setCursor(20, 25);
	tft.println("Enter Password");
	
	// Show SSID
	tft.setTextSize(1);
	tft.setCursor(20, 55);
	tft.print("Network: ");
	tft.println(targetSSID);
	
	// Password input box
	tft.drawRect(20, 85, 230, 30, TFT_WHITE);
	tft.fillRect(21, 86, 228, 28, TFT_BLACK);
	tft.setCursor(25, 93);
	tft.setTextColor(TFT_YELLOW);
	
	// Show password
	if (showPassword) {
		tft.println(inputPassword);
	} else {
		String maskedPassword = "";
		for (int i = 0; i < inputPassword.length(); i++) {
			maskedPassword += "*";
		}
		tft.println(maskedPassword);
	}
	
	// Show password checkbox
	tft.drawRect(255, 85, 20, 20, TFT_WHITE);
	if (showPassword) {
		tft.fillRect(258, 88, 14, 14, TFT_GREEN);
	}
	tft.setTextColor(TFT_WHITE);
	tft.setCursor(280, 90);
	tft.print("Show");
	
	// Virtual keyboard layout
	String keys[4][12] = {
		{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "@", "."},
		{"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "-", "_"},
		{"a", "s", "d", "f", "g", "h", "j", "k", "l", "#", "&", "DEL"},
		{"z", "x", "c", "v", "b", "n", "m", "!", "SPC", "OK", "CANCEL", ""}
	};
	
	int keyWidth = 25;
	int keyHeight = 25;
	int startX = 5;
	int startY = 135;
	
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 12; col++) {
			if (keys[row][col] == "") continue;
			
			int x = startX + col * (keyWidth + 2);
			int y = startY + row * (keyHeight + 3);
			
			// Special button colors
			if (keys[row][col] == "OK") {
				drawButton(x, y, keyWidth + 10, keyHeight, TFT_GREEN, keys[row][col]);
			} else if (keys[row][col] == "CANCEL") {
				drawButton(x, y, keyWidth + 15, keyHeight, TFT_RED, keys[row][col]);
			} else if (keys[row][col] == "DEL") {
				drawButton(x, y, keyWidth + 5, keyHeight, TFT_ORANGE, keys[row][col]);
			} else if (keys[row][col] == "SPC") {
				drawButton(x, y, keyWidth + 5, keyHeight, TFT_BLUE, keys[row][col]);
			} else {
				drawButton(x, y, keyWidth, keyHeight, TFT_DARKGREY, keys[row][col]);
			}
		}
	}
}

void drawConnectionStatus() {
	// Clear status area
	tft.fillRect(225, 200, 95, 30, TFT_BLACK);
	tft.setTextColor(TFT_WHITE);
	tft.setTextFont(1);
	tft.setTextSize(1);
	tft.setCursor(225, 205);
	
	if (WiFi.status() == WL_CONNECTED) {
		tft.setTextColor(TFT_GREEN);
		tft.print(WiFi.SSID());
		tft.setCursor(225, 215);
		tft.print("RSSI:");
		tft.print(WiFi.RSSI());
		tft.print("dBm");
		
		// Show disconnect button
		drawButton(120, 200, 100, 30, TFT_RED, "DISCONNECT");
	} else {
		tft.setTextColor(TFT_RED);
		tft.print("Disconnected");
		
		// Hide disconnect button
		tft.fillRect(120, 200, 100, 30, TFT_BLACK);
	}
}
