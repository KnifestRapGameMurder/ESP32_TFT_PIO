/**
 * @file main.cpp
 * @brief Media Controller for ESP32-2432S028 (CYD 2.8")
 */
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "CYD28_TouchscreenR.h"
#include "BleKeyboard.h"

TFT_eSPI tft = TFT_eSPI();
CYD28_TouchR touch(320, 240);
BleKeyboard bleKeyboard("CYD28_MediaController", "ESP32", 100);

bool bluetoothConnected = false;
String bluetoothStatus = "Disconnected";

void updateBluetoothStatus()
{
	tft.fillRect(10, 30, 300, 15, TFT_BLACK);
	tft.setTextColor(bluetoothConnected ? TFT_GREEN : TFT_RED);
	tft.setTextFont(1);
	tft.setCursor(10, 30);
	tft.print("BT Status: ");
	tft.print(bluetoothStatus);
}

void drawButton(int x, int y, int w, int h, uint16_t color, String text)
{
	// Draw button
	tft.fillRect(x, y, w, h, color);
	tft.drawRect(x, y, w, h, TFT_WHITE);
	
	// Draw text
	tft.setTextColor(TFT_BLACK);
	tft.setTextFont(1);
	int textX = x + (w - text.length() * 6) / 2;
	int textY = y + (h - 8) / 2;
	tft.setCursor(textX, textY);
	tft.print(text);
}

void drawIconButton(int x, int y, int w, int h, uint16_t color, String icon, String label)
{
	// Draw button
	tft.fillRect(x, y, w, h, color);
	tft.drawRect(x, y, w, h, TFT_WHITE);
	
	// Draw icon (larger text)
	tft.setTextColor(TFT_BLACK);
	tft.setTextFont(2);
	int iconX = x + (w - icon.length() * 12) / 2;
	int iconY = y + 8;
	tft.setCursor(iconX, iconY);
	tft.print(icon);
	
	// Draw label (smaller text)
	tft.setTextFont(1);
	int labelX = x + (w - label.length() * 6) / 2;
	int labelY = y + h - 12;
	tft.setCursor(labelX, labelY);
	tft.print(label);
}

bool isButtonPressed(int touchX, int touchY, int btnX, int btnY, int btnW, int btnH)
{
	return (touchX >= btnX && touchX <= (btnX + btnW) && 
			touchY >= btnY && touchY <= (btnY + btnH));
}

void showAction(String action)
{
	tft.fillRect(10, 210, 300, 15, TFT_BLACK);
	tft.setTextColor(TFT_CYAN);
	tft.setTextFont(1);
	tft.setCursor(10, 210);
	tft.print("Action: ");
	tft.print(action);
}

void setup()
{
	Serial.begin(115200);
	Serial.println("Media Controller Starting...");

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
	tft.print("Media Controller");

	// Initialize Bluetooth
	Serial.println("Starting BLE Keyboard...");
	bleKeyboard.begin();
	bluetoothStatus = "Pairing...";
	updateBluetoothStatus();

	// Draw media control buttons (Top Row - Larger buttons)
	drawIconButton(20, 60, 90, 60, TFT_GREEN, "|>", "PLAY/PAUSE");
	drawIconButton(120, 60, 80, 60, TFT_BLUE, "<<", "PREV");
	drawIconButton(210, 60, 80, 60, TFT_CYAN, ">>", "NEXT");

	// Draw volume control buttons (Middle Row)
	drawIconButton(40, 130, 100, 50, TFT_RED, "-", "VOL DOWN");
	drawIconButton(150, 130, 100, 50, TFT_ORANGE, "+", "VOL UP");
	drawIconButton(260, 130, 60, 50, TFT_PURPLE, "M", "MUTE");

	// Draw system control buttons (Bottom Row)
	drawButton(80, 190, 160, 30, TFT_MAGENTA, "DISCONNECT BT");

	Serial.println("Setup complete. Ready for media control!");
	Serial.println("Pair with 'CYD28_MediaController' on your device");
}

void loop()
{
	// Update connection status
	if(bleKeyboard.isConnected() != bluetoothConnected)
	{
		bluetoothConnected = bleKeyboard.isConnected();
		bluetoothStatus = bluetoothConnected ? "Connected" : "Pairing...";
		updateBluetoothStatus();
		
		if(bluetoothConnected)
		{
			Serial.println("BLE Keyboard Connected!");
			showAction("Connected to device");
		}
		else
		{
			Serial.println("BLE Keyboard Disconnected!");
			showAction("Disconnected");
		}
	}

	if (touch.touched())
	{
		CYD28_TS_Point p = touch.getPointScaled();
		
		Serial.print("Touch at: ");
		Serial.print(p.x);
		Serial.print(", ");
		Serial.println(p.y);

		if(bleKeyboard.isConnected())
		{
			// Media Control Buttons (Top Row)
			if (isButtonPressed(p.x, p.y, 20, 60, 90, 60)) {
				Serial.println("PLAY/PAUSE button pressed!");
				bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
				showAction("Play/Pause");
			}
			else if (isButtonPressed(p.x, p.y, 120, 60, 80, 60)) {
				Serial.println("PREVIOUS button pressed!");
				bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
				showAction("Previous Track");
			}
			else if (isButtonPressed(p.x, p.y, 210, 60, 80, 60)) {
				Serial.println("NEXT button pressed!");
				bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
				showAction("Next Track");
			}
			
			// Volume Control Buttons (Middle Row) - with hold functionality
			else if (isButtonPressed(p.x, p.y, 40, 130, 100, 50)) {
				Serial.println("VOLUME DOWN button pressed!");
				bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
				showAction("Volume Down");
				
				// Hold functionality - repeat while pressed
				unsigned long holdStartTime = millis();
				while (touch.touched() && isButtonPressed(touch.getPointScaled().x, touch.getPointScaled().y, 40, 130, 100, 50)) {
					if (millis() - holdStartTime > 500) { // Start repeating after 500ms
						bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
						delay(100); // Repeat every 100ms while held
					}
					delay(10);
				}
			}
			else if (isButtonPressed(p.x, p.y, 150, 130, 100, 50)) {
				Serial.println("VOLUME UP button pressed!");
				bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
				showAction("Volume Up");
				
				// Hold functionality - repeat while pressed
				unsigned long holdStartTime = millis();
				while (touch.touched() && isButtonPressed(touch.getPointScaled().x, touch.getPointScaled().y, 150, 130, 100, 50)) {
					if (millis() - holdStartTime > 500) { // Start repeating after 500ms
						bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
						delay(100); // Repeat every 100ms while held
					}
					delay(10);
				}
			}
			else if (isButtonPressed(p.x, p.y, 260, 130, 60, 50)) {
				Serial.println("MUTE button pressed!");
				bleKeyboard.write(KEY_MEDIA_MUTE);
				showAction("Mute Toggle");
			}

			// System Control Button (Bottom Row)
			else if (isButtonPressed(p.x, p.y, 80, 190, 160, 30)) {
				Serial.println("DISCONNECT BT button pressed!");
				// End the BLE connection
				bleKeyboard.end();
				delay(1000);
				// Restart BLE to allow reconnection
				bleKeyboard.begin();
				bluetoothStatus = "Pairing...";
				updateBluetoothStatus();
				showAction("Bluetooth Restarted");
			}
		}
		else
		{
			// Show connection message if not connected
			if (isButtonPressed(p.x, p.y, 20, 60, 270, 120)) {
				showAction("Not connected - pair device first");
			}
		}

		// Wait for release (skip for volume buttons as they handle their own hold logic)
		if (!isButtonPressed(p.x, p.y, 40, 130, 100, 50) && !isButtonPressed(p.x, p.y, 150, 130, 100, 50)) {
			while (touch.touched()) {
				delay(10);
			}
		}
		delay(100);
	}
	
	delay(20);
}
