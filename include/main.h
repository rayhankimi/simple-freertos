#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <DHT.h>
#include <ezButton.h>

struct Button {
  uint8_t pin;
  ezButton button;
};

float temperature;
float humidity;

void updateButtonStates();
void initializeButtons();
void handleSensorTask(void *parameter);
void handleDisplayTask(void *parameter);
void setup();
void loop();