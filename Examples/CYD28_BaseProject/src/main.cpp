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

void displayNetworks();

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
	Serial.println("WiFi Manager Starting...");

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
	tft.setCursor(10, 5);
	tft.print("WiFi Manager");

	// Initialize WiFi UI
	wifi_ui_init();

	// Draw SCAN button
	drawButton(240, 35, 70, 30, TFT_GREEN, "SCAN");

	Serial.println("Setup complete. Ready for WiFi management!");
}

void loop()
{
	if (touch.touched())
	{
		CYD28_TS_Point p = touch.getPointScaled();
		
		Serial.print("Touch at: ");
		Serial.print(p.x);
		Serial.print(", ");
		Serial.println(p.y);

		// SCAN button
		if (isButtonPressed(p.x, p.y, 240, 35, 70, 30)) {
			Serial.println("SCAN button pressed!");
			wifi_ui_scan();
			
			// Show action
			tft.fillRect(10, 200, 300, 15, TFT_BLACK);
			tft.setTextColor(TFT_CYAN);
			tft.setTextFont(1);
			tft.setCursor(10, 200);
			tft.print("Scanning...");
			
			displayNetworks();
		}

		// Wait for release
		while (touch.touched()) {
			delay(10);
		}
		delay(100);
	}
	
	delay(20);
}

void displayNetworks() {
	// Clear network area
	tft.fillRect(10, 70, 300, 120, TFT_BLACK);
	
	int count = wifi_ui_get_network_count();
	tft.setTextColor(TFT_WHITE);
	tft.setTextFont(1);
	
	for (int i = 0; i < count && i < 8; i++) {
		int y = 70 + (i * 15);
		tft.setCursor(10, y);
		
		// Show lock icon for encrypted networks
		if (wifi_ui_get_encrypted(i)) {
			tft.print("[L] ");
		} else {
			tft.print("[ ] ");
		}
		
		// Show SSID
		tft.print(wifi_ui_get_ssid(i));
		
		// Show signal strength
		int rssi = wifi_ui_get_rssi(i);
		tft.setCursor(250, y);
		if (rssi > -50) tft.print("****");
		else if (rssi > -60) tft.print("*** ");
		else if (rssi > -70) tft.print("**  ");
		else tft.print("*   ");
	}
}
