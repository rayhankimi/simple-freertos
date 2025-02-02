#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <DHT.h>
#include <ezButton.h>

#include <main.h>

#define DHT_TYPE DHT22
#define DHT_PIN 33
#define BTN_NUM 3
#define BTN_1 15
#define BTN_2 2
#define BTN_3 4
#define DEBOUNCE_DELAY 50

Button buttons[BTN_NUM] = {
    {BTN_1, ezButton(BTN_1)},
    {BTN_2, ezButton(BTN_2)},
    {BTN_3, ezButton(BTN_3)},
};

DHT dht(DHT_PIN, DHT_TYPE);

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  initializeButtons();
  dht.begin();

  xTaskCreatePinnedToCore(
      handleSensorTask, // Task function
      "Sensor Task",    // Task name
      4096,             // Stack size
      NULL,             // Parameter
      2,                // High priority
      NULL,             // Task handle
      0                 // Core 0
  );

  xTaskCreatePinnedToCore(
      handleDisplayTask, // Task function
      "Display Task",    // Task name
      4096,              // Stack size
      NULL,              // Parameter
      1,                 // High priority
      NULL,              // Task handle
      0                  // Core 0
  );
}

void loop()
{
  // put your main code here, to run repeatedly:
}

void handleSensorTask(void *parameter)
{
  for (;;)
  {
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
}

void handleDisplayTask(void *parameter)
{
  for (;;)
  {
    updateButtonStates();
    if (buttons[0].button.isPressed())
    {
      Serial.print("Temperature:");
      Serial.println(temperature);
    }
    if (buttons[1].button.isPressed())
    {
      Serial.println("Button 2 pressed");
    }
    if (buttons[2].button.isPressed())
    {
      Serial.print("Humidity: ");
      Serial.println(humidity);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void initializeButtons()
{
  for (int i = 0; i < BTN_NUM; i++)
  {
    buttons[i].button.setDebounceTime(DEBOUNCE_DELAY);
  }
}

void updateButtonStates()
{
  for (int i = 0; i < BTN_NUM; i++)
  {
    buttons[i].button.loop();
  }
}
