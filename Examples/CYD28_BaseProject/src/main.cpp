/**
 * @file main.cpp
 * @brief Simple button test for ESP32-2432S028 (CYD 2.8")
 */
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "CYD28_TouchscreenR.h"

TFT_eSPI tft = TFT_eSPI();
CYD28_TouchR touch(320, 240);

int buttonPressCount = 0;

void updateCountDisplay()
{
	tft.fillRect(250, 25, 70, 15, TFT_BLACK);
	tft.setTextColor(TFT_WHITE);
	tft.setTextFont(1);
	tft.setCursor(250, 25);
	tft.print("Count: ");
	tft.print(buttonPressCount);
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

bool isButtonPressed(int touchX, int touchY, int btnX, int btnY, int btnW, int btnH)
{
	return (touchX >= btnX && touchX <= (btnX + btnW) &&
			touchY >= btnY && touchY <= (btnY + btnH));
}

void setup()
{
	Serial.begin(115200);
	Serial.println("Simple Button Test");

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
	tft.setCursor(10, 10);
	tft.print("Button Test");

	// Draw count display
	updateCountDisplay();

	// Draw buttons
	drawButton(20, 60, 80, 40, TFT_RED, "RED");
	drawButton(120, 60, 80, 40, TFT_GREEN, "GREEN");
	drawButton(220, 60, 80, 40, TFT_BLUE, "BLUE");

	drawButton(20, 120, 80, 40, TFT_YELLOW, "YELLOW");
	drawButton(120, 120, 80, 40, TFT_MAGENTA, "MAGENTA");
	drawButton(220, 120, 80, 40, TFT_CYAN, "CYAN");

	drawButton(70, 180, 80, 30, TFT_WHITE, "CLEAR");
	drawButton(170, 180, 80, 30, TFT_ORANGE, "COUNT");

	Serial.println("Touch buttons to test!");
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

		// Check button presses
		if (isButtonPressed(p.x, p.y, 20, 60, 80, 40))
		{
			Serial.println("RED button pressed!");
			tft.fillRect(10, 40, 200, 15, TFT_BLACK);
			tft.setTextColor(TFT_RED);
			tft.setTextFont(1);
			tft.setCursor(10, 40);
			tft.print("RED pressed!");
		}
		else if (isButtonPressed(p.x, p.y, 120, 60, 80, 40))
		{
			Serial.println("GREEN button pressed!");
			tft.fillRect(10, 40, 200, 15, TFT_BLACK);
			tft.setTextColor(TFT_GREEN);
			tft.setTextFont(1);
			tft.setCursor(10, 40);
			tft.print("GREEN pressed!");
		}
		else if (isButtonPressed(p.x, p.y, 220, 60, 80, 40))
		{
			Serial.println("BLUE button pressed!");
			tft.fillRect(10, 40, 200, 15, TFT_BLACK);
			tft.setTextColor(TFT_BLUE);
			tft.setTextFont(1);
			tft.setCursor(10, 40);
			tft.print("BLUE pressed!");
		}
		else if (isButtonPressed(p.x, p.y, 20, 120, 80, 40))
		{
			Serial.println("YELLOW button pressed!");
			tft.fillRect(10, 40, 200, 15, TFT_BLACK);
			tft.setTextColor(TFT_YELLOW);
			tft.setTextFont(1);
			tft.setCursor(10, 40);
			tft.print("YELLOW pressed!");
		}
		else if (isButtonPressed(p.x, p.y, 120, 120, 80, 40))
		{
			Serial.println("MAGENTA button pressed!");
			tft.fillRect(10, 40, 200, 15, TFT_BLACK);
			tft.setTextColor(TFT_MAGENTA);
			tft.setTextFont(1);
			tft.setCursor(10, 40);
			tft.print("MAGENTA pressed!");
		}
		else if (isButtonPressed(p.x, p.y, 220, 120, 80, 40))
		{
			Serial.println("CYAN button pressed!");
			tft.fillRect(10, 40, 200, 15, TFT_BLACK);
			tft.setTextColor(TFT_CYAN);
			tft.setTextFont(1);
			tft.setCursor(10, 40);
			tft.print("CYAN pressed!");
		}
		else if (isButtonPressed(p.x, p.y, 70, 180, 80, 30))
		{
			Serial.println("CLEAR button pressed!");
			tft.fillRect(10, 40, 200, 15, TFT_BLACK);
			buttonPressCount = 0;
			updateCountDisplay();
		}
		else if (isButtonPressed(p.x, p.y, 170, 180, 80, 30))
		{
			Serial.println("COUNT button pressed!");
			buttonPressCount++;
			updateCountDisplay();
		}

		// Wait for release
		while (touch.touched())
		{
			delay(10);
		}
		delay(100);
	}

	delay(20);
}
